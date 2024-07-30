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
  * @brief Implement fat image settings.
  */

 #include <config.h>
 #include <udjat/tools/xml.h>
 #include <memory>

 #ifdef HAVE_FATFS

 #include <fatfs.h>
 #include <fatfs/ff.h>

 using namespace Udjat;
 using namespace std;

 namespace FatFS {

	// Reference: http://elm-chan.org/fsw/ff/doc/mkfs.html

	Image::Settings::Settings(const Udjat::XML::Node &node)
		: type{FM_ANY}, n_fats{(uint8_t) XML::AttributeFactory(node,"n_fats").as_uint(1)}, align{XML::AttributeFactory(node,"align").as_uint(0)}, n_root{XML::AttributeFactory(node,"n_root").as_uint(0)}, au_size{XML::AttributeFactory(node,"au_size").as_uint(0)} {
	}

	size_t Image::Settings::fat_length() const noexcept {

		// Reference: http://elm-chan.org/fsw/ff/res/mkfs.xlsx
		unsigned int csize = 512; // TODO: Get the real value.

		if(type == FM_ANY) {
			if(au_size == 4096) {
				return 8 * n_fats * csize;
			}
			if(au_size == 0) {
				return 1 * n_fats * csize;
			}
		}

		if(type == FM_FAT) {
			if(au_size == 0) {
				return 1 * n_fats * csize;
			}
		}

		// Default
		return 8 * n_fats * csize;

	}

 }
 #endif // HAVE_FATFS

