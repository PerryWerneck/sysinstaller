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

	Image::Image(const Settings &s) : settings{s} {

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

			if(settings.system_area && *settings.system_area) {
				fd = open(settings.system_area,O_RDONLY);
			} else {
				fd = open(Config::Value<string>("iso9660","system-area","/usr/share/syslinux/isohdpfx.bin").c_str(),O_RDONLY);
			}

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
				iso_image_set_application_id(image,Config::Value<string>("iso9660","application-id",Application::Name().c_str()).c_str());;
			}
		}

	}

	Image::~Image() {

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

	void Image::append(const char *from, const char *to) {

		if(*to == '.') {
			to++;
		}

		debug("Appending '",to,"'");

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

	void Image::pre(Udjat::Abstract::Object &object) {

		/*
		if(efibootimage->enabled()) {
			efibootimage->build(object);
		}
		*/

	}

	void Image::post(Udjat::Abstract::Object &object) {

		iso_write_opts_set_rockridge(opts, settings.rockridge);
		iso_write_opts_set_joliet(opts, settings.joliet);
		iso_write_opts_set_allow_deep_paths(opts, settings.allow_deep_paths);

		if(settings.boot.eltorito) {

			// Search to confirm presence of the boot_image.
			/*
			const char *filename = action->source(action->boot.eltorito.image)->filename();
			if(!(filename && *filename)) {
				throw runtime_error(_("Unexpected filename on el-torito boot image"));
			}
			*/

			/*
			set_el_torito_boot_image(
				action->boot.eltorito.image, isopath
				action->boot.catalog, catalog
				action->volume_id id
			);
			*/
			{
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
				iso_write_opts_set_part_like_isohybrid(opts, 1);

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

			}

			Logger::String{"El-torito boot image set to '",settings.boot.eltorito.image,"'"}.trace("iso9660");

		}

		/*
		Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Setting up ISO image"));

		set_rockridge();
		set_joliet();
		set_allow_deep_paths();

		if(action->boot.eltorito) {

			Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding el-torito boot image"));

			// Search to confirm presence of the boot_image.
			const char *filename = action->source(action->boot.eltorito.image)->filename();
			if(!(filename && *filename)) {
				throw runtime_error(_("Unexpected filename on el-torito boot image"));
			}

			set_el_torito_boot_image(
				action->boot.eltorito.image,
				action->boot.catalog,
				action->volume_id
			);

			cout << "iso9660\tEl-torito boot image set to '" << action->boot.eltorito.image << "'" << endl;
		}

		if(action->boot.efi->enabled()) {

			Reinstall::Dialog::Progress::getInstance().set_sub_title(_("Adding EFI boot image"));

			auto source = action->source(action->boot.efi->path());
			const char *filename = source->filename(true);
			if(!filename[0]) {
				throw runtime_error(_("Unexpected filename on EFI boot image"));
			}

			// Apply templates on EFI boot image.
			{
				debug("Applying templates on EFI boot image at '",filename,"'");

				Disk::Image disk(filename);

				for(auto tmpl : action->templates) {

					disk.forEach([this,&tmpl](const char *mountpoint, const char *path){

						if(tmpl->test(path)) {
							tmpl->load((Udjat::Object &) *this);
							cout << "efi\tReplacing " << path << " with template " << tmpl->c_str() << endl;
							tmpl->replace((string{mountpoint} + "/" + path).c_str());
						}

					});

				}
			}

			// Add EFI boot image
			Logger::String{"Adding ",filename," as EFI boot image"}.info(name);
			set_efi_boot_image(filename);

			if(action->boot.catalog && *action->boot.catalog) {
				Logger::String{"Adding ",source->path," as boot image"}.info(name);
				add_boot_image(source->path,0xEF);
			} else {
				Logger::String{"No boot catalog, ",source->path," was not added as boot image"}.trace(name);
			}

		}

		*/
	}

 }


