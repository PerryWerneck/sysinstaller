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

 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 #define LIBISOFS_WITHOUT_LIBBURN
 #include <libisofs/libisofs.h>

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

 }


