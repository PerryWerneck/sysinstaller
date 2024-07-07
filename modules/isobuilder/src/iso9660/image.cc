/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 /**
  * @brief Implements iso9660 image.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <iso9660.h>
 #include <string>

 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>

 #include <reinstall/disk/fat.h>
 #include <reinstall/tools/builder.h>

 #define LIBISOFS_WITHOUT_LIBBURN
 #include <libisofs/libisofs.h>

 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace iso9660 {

	class UDJAT_PRIVATE IsoBuilderSingleTon {
	private:
		IsoBuilderSingleTon() {
			cout << "iso9660\tStarting iso builder" << endl;
			iso_init();
		}

	public:
		static IsoBuilderSingleTon &getInstance() {
			static IsoBuilderSingleTon instance;
			return instance;
		}

		~IsoBuilderSingleTon() {
			iso_finish();
			cout << "iso9660\tIso builder was terminated" << endl;
		}

	};

	Image::Image(Reinstall::Builder &builder, const Settings &s) : Reinstall::Abstract::Image{builder,"iso-9660"}, settings{s} {

		IsoBuilderSingleTon::getInstance();

		if(!iso_image_new("name", &image)) {
			throw runtime_error(_("Error creating iso image"));
		}

		iso_image_attach_data(image,this,NULL);

		iso_write_opts_new(&opts, 2);
		iso_write_opts_set_relaxed_vol_atts(opts, 1);
		iso_write_opts_set_rrip_version_1_10(opts,1);

		// Set system area
		{
			char data[32768];
			memset(data,0,sizeof(data));

			int fd;

			string filename;
			if(settings.system_area && *settings.system_area) {
				filename = settings.system_area;
			} else {
				filename = Config::Value<string>("iso9660","system-area","/usr/share/syslinux/isohdpfx.bin");
			}

			Logger::String{"Loading system area from '",filename.c_str(),"'"}.trace("iso9660");
			fd = open(filename.c_str(),O_RDONLY);
			if(fd <  0) {
				throw system_error(errno, system_category(), _("Error loading system area"));
			}

			try {

				if(read(fd,data,sizeof(data)) < 1) {
					throw system_error(errno, system_category(), _("Cant read system area definition file"));
				}

				int rc = iso_write_opts_set_system_area(opts,data,2,0);
				if(rc != ISO_SUCCESS) {
					throw runtime_error(iso_error_to_msg(rc));
				}

			} catch(...) {
				::close(fd);
				throw;
			}

			::close(fd);

		}

		// Set volume id
		{
			if(settings.volume_id && *settings.volume_id) {
				iso_image_set_volume_id(image, settings.volume_id);
			} else {
				iso_image_set_volume_id(image, Config::Value<string>("iso9660","volume-id",Application::Name().c_str()).c_str());
			}
		}

		// set publisher id
		{
			if(settings.publisher_id && *settings.publisher_id) {
				iso_image_set_publisher_id(image, settings.publisher_id);
			} else {
				iso_image_set_publisher_id(image, Config::Value<string>{"iso9660","publisher-id",Application::Name().c_str()}.c_str());
			}
		}

		// set data preparer id
		{
			if(settings.data_preparer_id && *settings.data_preparer_id) {

				iso_image_set_data_preparer_id(image, settings.data_preparer_id);

			} else {

				char username[32];
				if(getlogin_r(username, 32) == 0) {
					iso_image_set_data_preparer_id(image, username);
				}

			}
		}

		// set system id
		{
			if(settings.system_id && *settings.system_id) {
				iso_image_set_system_id(image,settings.system_id);
			} else {
				iso_image_set_system_id(image,Config::Value<string>("iso9660","system-id","LINUX").c_str());;
			}
		}

		// set application id
		{
			if(settings.application_id && *settings.application_id) {
				iso_image_set_application_id(image,settings.application_id);
			} else {
				iso_image_set_application_id(image,Config::Value<string>("iso9660","application-id",Reinstall::Abstract::Image::application_id()).c_str());;
			}
		}

	}

	Image::~Image() {

		if(!empty(efibootpart)) {
			unlink(efibootpart.c_str());
		}

		iso_image_unref(image);
		iso_write_opts_free(opts);

	}

	static IsoDir * getIsoDir(IsoImage *image, const char *dirname) {

		if(*dirname == '/') {
			dirname++;
		}

		int rc;
		IsoDir * dir = iso_image_get_root(image);

		for(auto dn : Udjat::String(dirname).split("/")) {

			IsoNode *node;
			rc = iso_image_dir_get_node(image,dir,dn.c_str(),&node,0);
			if(rc == 0) {

				// Not found, add it.
				rc = iso_tree_add_new_dir(dir, dn.c_str(), (IsoDir **) &node);

			};

			if(rc < 0) {
				std::string msg{iso_error_to_msg(rc)};
				Logger::String{"Error '",msg.c_str(),"' adding path ",dirname}.error("iso-9660");
				throw runtime_error(msg);
			}

			dir = (IsoDir *) node;

		}

		return dir;

	}

	static void add_new_node(IsoImage *image, const char *from, const char *to) {

		int rc = -1;

		auto pos = strrchr(to,'/');
		if(pos) {

			if(!*(pos+1)) {
				Logger::String{"Can't insert node '",to,"' it's not a FILE name, looks like a DIRECTORY name"}.error("iso9660");
				throw logic_error(_("Unexpanded path in source list"));
			}

			// Has path, get iso dir.
			rc = iso_tree_add_new_node(
				image,
				getIsoDir(image,string(to,pos - to).c_str()),
				pos+1,
				from,
				NULL
			);

		} else {

			// No path, store on root.
			rc = iso_tree_add_new_node(
				image,
				iso_image_get_root(image),
				to,
				from,
				NULL
			);

		}

		if(rc < 0) {
			string msg{iso_error_to_msg(rc)};
			Logger::String{"Error '",msg.c_str(),"' adding node ",to}.error("iso9660");
			throw runtime_error(msg);
		}

	}

	void Image::append(const char *from, const char *to) {

		if(*to == '.') {
			to++;
		}

		/*
		#error Move EFI adjustments to Abstract::Image
		if(settings.boot.efi->enabled() && strcmp(to,settings.boot.efi->path()) == 0) {

			debug("------------------------------------------------- EFIBOOT");

			// Copy contents to temporary file.
			if(!empty(efibootpart)) {
				throw runtime_error("EFI boot partition already set");
			}
			efibootpart = Udjat::File::Temporary::create();

			Udjat::File::copy(from,efibootpart.c_str());
			from = efibootpart.c_str();

			// Apply templates in efibootpart
			{
				std::vector<string> files;

				Reinstall::Disk::Fat32 disk{from};

				disk.for_each("/",[&files](const char *filename){
					files.push_back(filename);
					return false;
				});

			}

		}
		*/

		add_new_node(image,from,to);

	}

	void Image::pre(Udjat::Abstract::Object &object) {
	}

	void Image::post(Udjat::Abstract::Object &object) {

		iso_write_opts_set_rockridge(opts, settings.rockridge);
		iso_write_opts_set_joliet(opts, settings.joliet);
		iso_write_opts_set_allow_deep_paths(opts, settings.allow_deep_paths);

		if(settings.boot.eltorito) {

			ElToritoBootImage *bootimg = NULL;

			Logger::String{"Setting el-torito boot image to '",settings.boot.eltorito.image,"'"}.trace("iso9660");
			Logger::String{"Setting boot catalog to '",settings.boot.catalog,"'"}.trace("iso9660");

			int rc = iso_image_set_boot_image(
							image,
							settings.boot.eltorito.image,
							ELTORITO_NO_EMUL,
							settings.boot.catalog,
							&bootimg
						);

			if(rc < 0) {
				string msg{iso_error_to_msg(rc)};
				Logger::String{"Error '",msg.c_str(),"' setting el-torito boot image"}.error("iso9660");
				throw runtime_error(msg);
			}

			el_torito_set_load_size(bootimg, 4);
			el_torito_patch_isolinux_image(bootimg);

			if(settings.like_iso_hybrid) {
				iso_write_opts_set_part_like_isohybrid(opts, 1);
			}

			{
				uint8_t id_string[28];
				memset(id_string,' ',sizeof(id_string));

				if(settings.boot.eltorito.id && *settings.boot.eltorito.id) {
					size_t len = strlen(settings.boot.eltorito.id);
					if(len > 28) {
						len = 28;
					}
					strncpy((char *) id_string,settings.boot.eltorito.id,len);
				} else {
					Config::Value<string> defstring("iso9660","el-torito-id",Application::Name().c_str());
					strncpy((char *) id_string,defstring,strlen(defstring));
				}

				el_torito_set_id_string(bootimg,id_string);
				Logger::String{"El-torito ID string set to '",string((const char *) id_string,28).c_str(),"'"}.trace("iso9660");
			}

			// bit0= Patch the boot info table of the boot image. This does the same as mkisofs option -boot-info-table.
			el_torito_set_isolinux_options(bootimg,1,0);

			Logger::String{"El-torito boot image set to '",settings.boot.eltorito.image,"'"}.trace("iso9660");

		}

		if(!empty(efibootpart)) {

			// Add EFI boot image
			Logger::String{"Adding ",efibootpart.c_str()," as EFI boot image"}.write(Logger::Debug,"iso9660");

			// set_efi_boot_image(const char *boot_image, bool like_iso_hybrid)
			// set_efi_boot_image(settings.boot.efi->path().c_str());
			if(settings.like_iso_hybrid) {

				iso_write_opts_set_part_like_isohybrid(opts, 1);

				// Isohybrid, set partition
				int rc = iso_write_opts_set_partition_img(opts,2,0xef,(char *) efibootpart.c_str(),0);

				if(rc != ISO_SUCCESS) {
					string msg{iso_error_to_msg(rc)};
					Logger::String{"Cant set EFI partition: ",msg.c_str()}.error("iso9660");
					throw runtime_error(msg);
				}

				// Logger::String{"EFI partition set from '",efibootpart.c_str(),"'"}.trace("iso9660");

			} else {

				// Not isohybrid.
				int rc = iso_write_opts_set_efi_bootp(opts,(char *) builder.efi()->path(),0);

				if(rc != ISO_SUCCESS) {
					string msg{iso_error_to_msg(rc)};
					Logger::String{"Cant set EFI bootp: ",msg.c_str()}.error("iso9660");
					throw runtime_error(msg);
				}

				Logger::String{"EFI bootp set from '",builder.efi()->path(),"'"}.write(Logger::Debug,"iso9660");

			}

			if(settings.boot.catalog && *settings.boot.catalog) {

				Logger::String{"Adding ",builder.efi()->path()," as boot image"}.write(Logger::Debug,"iso9660");

                ElToritoBootImage *bootimg = NULL;
                int rc = iso_image_add_boot_image(image,builder.efi()->path(),ELTORITO_NO_EMUL,0,&bootimg);
                if(rc < 0) {
 					string msg{iso_error_to_msg(rc)};
					Logger::String{"Cant add EFI boot image: ",msg.c_str()}.error("iso9660");
					throw runtime_error(msg);
				}

				el_torito_set_boot_platform_id(bootimg, 0xEF);

			} else {

				Logger::String{"No boot catalog, ",builder.efi()->path()," was not added as boot image"}.warning("iso9660");

			}

		} else if(builder.efi()->enabled()) {

			throw runtime_error("EFI boot image not available");

		}

	}

	void Image::write(Udjat::Dialog::Progress &dialog, const std::function<void(unsigned long long offset, const void *contents, unsigned long long length)> &write) {

		dialog = _( "Preparing to write" );

		int rc = iso_image_update_sizes(image);
		if (rc < 0) {
			string msg{iso_error_to_msg(rc)};
			Logger::String{"Error updating image size: ",msg.c_str()}.error("iso9660");
			throw runtime_error(msg);
		}

		struct burn_source *burn_src = NULL;
		rc = iso_image_create_burn_source(image, opts, &burn_src);
		if (rc < 0) {
			string msg{iso_error_to_msg(rc)};
			Logger::String{"Error creating burn source: ",msg.c_str()}.error("iso9660");
			throw runtime_error(msg);
		}

		dialog = _( "Writing image" );

		try {

			#define BUFLEN 2048
			unsigned char buffer[BUFLEN];

			unsigned long long current = 0;
			unsigned long long total = burn_src->get_size(burn_src);

			while(burn_src->read_xt(burn_src, buffer, BUFLEN) == BUFLEN) {
				write(current, buffer, BUFLEN);
				current += BUFLEN;
				if(total) {
					dialog = ((double) total) / ((double) current);
				}
			}

		} catch(...) {

				burn_src->free_data(burn_src);
				free(burn_src);
				throw;

		}

		burn_src->free_data(burn_src);
		free(burn_src);

	}

 }


