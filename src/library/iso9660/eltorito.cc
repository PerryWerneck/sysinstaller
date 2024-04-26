/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implement el-torito methods.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <libreinstall/iso9660.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace iso9660 {

#if UDJAT_CHECK_VERSION(1,2,0)
	Settings::Boot::ElTorito::ElTorito(const XML::Node &node) :
		image{XML::QuarkFactory(node,"el-torito-boot-image","/boot/x86_64/loader/isolinux.bin")},
		id{XML::QuarkFactory(node,"el-torito-boot-id",PACKAGE_NAME)} {
	}
#else
	Settings::Boot::ElTorito::ElTorito(const XML::Node &node) :
		image{XML::QuarkFactory(node,"el-torito-boot-image","value","/boot/x86_64/loader/isolinux.bin").c_str()},
		id{XML::QuarkFactory(node,"el-torito-boot-id","value",PACKAGE_NAME).c_str()} {
	}
#endif // UDJAT_CHECK_VERSION

	void Image::set_bootable(const char *catalog, const Settings::Boot::ElTorito &boot) {

		if(!(boot.image && *boot.image)) {
			throw runtime_error("Invalid el-torito boot image name");
		}

		ElToritoBootImage *bootimg = NULL;
		int rc = iso_image_set_boot_image(image,boot.image,ELTORITO_NO_EMUL,catalog,&bootimg);
		if(rc < 0) {
			Logger::String{"Cant set ",boot.image," as el-torito boot image: ",iso_error_to_msg(rc)}.error("iso9660");
			throw runtime_error(iso_error_to_msg(rc));
		}

		Logger::String{"El-torito boot image set from '",boot.image,"'"}.trace("iso9660");

		el_torito_set_load_size(bootimg, 4);
		el_torito_patch_isolinux_image(bootimg);

		if(boot.isohybrid) {
			set_part_like_isohybrid();
		}

		{
			uint8_t id_string[28];
			memset(id_string,' ',sizeof(id_string));

			if(boot.id && *boot.id) {
				strncpy((char *) id_string,boot.id,strlen(boot.id));
			} else {
				Config::Value<string> defstring("iso9660","el-torito-id",PACKAGE_NAME);
				strncpy((char *) id_string,defstring,strlen(defstring));
			}

			el_torito_set_id_string(bootimg,id_string);
			Logger::String{"El-torito ID string set to '",string{(const char *) id_string,28},"'"}.trace("iso9660");

		}

		// bit0= Patch the boot info table of the boot image. This does the same as mkisofs option -boot-info-table.
		el_torito_set_isolinux_options(bootimg,1,0);

	}

 }
