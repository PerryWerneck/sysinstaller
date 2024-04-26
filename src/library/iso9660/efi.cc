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
  * @brief Implement EFI methods.
  */

 #include <config.h>

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <libreinstall/iso9660.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace iso9660 {

#if UDJAT_CHECK_VERSION(1,2,0)
	Settings::Boot::Efi::Efi(const XML::Node &node)
		: image{XML::QuarkFactory(node,"efi-boot-image","/boot/x86_64/efi")} {
	}
#else
	Settings::Boot::Efi::Efi(const XML::Node &node)
		: image{XML::QuarkFactory(node,"efi-boot-image","value","/boot/x86_64/efi").c_str()} {
	}
#endif // UDJAT_CHECK_VERSION

	void Image::set_bootable(const char *partdata, const Settings::Boot::Efi &boot) {

		if(!(partdata && *partdata)) {
			throw runtime_error("Invalid EFI partition data");
		}

		// Set EFI boot partition.
		if(boot.isohybrid) {

			// iso-hybrid, set partition
			set_part_like_isohybrid();
			int rc = iso_write_opts_set_partition_img(opts,boot.partition_number,boot.partition_type,(char *) partdata,0);

			if(rc != ISO_SUCCESS) {
				Logger::String{"Cant set ",partdata," as efi boot image: ",iso_error_to_msg(rc)}.error("iso9660");
				throw runtime_error(iso_error_to_msg(rc));
			}

			Logger::String{"EFI partition ",boot.partition_number," set from '",partdata,"' (isohybrid)"}.trace("iso9660");

		} else {

			// Non iso-hybrid, set bootp.
			int rc = iso_write_opts_set_efi_bootp(opts,(char *) partdata,0);

			if(rc != ISO_SUCCESS) {
				Logger::String{"Cant set ",partdata," as bootp image: ",iso_error_to_msg(rc)}.error("iso9660");
				throw runtime_error(iso_error_to_msg(rc));
			}

			Logger::String{"BOOTP partition set from '",partdata,"'"}.trace("iso9660");

		}


	}


 }
