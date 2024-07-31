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
  * @brief Implements fatfs image.
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

	Image::Image(const Udjat::Dialog &dialog, Reinstall::Builder &builder, const Settings &s) : Reinstall::Abstract::Image{dialog,builder}, settings{s} {

	}

	Image::~Image() {

	}

	void Image::append(std::shared_ptr<Reinstall::DataSource> source) {
	}

	void Image::append(const char *from, const char *to) {

		if(*to == '.') {
			to++;
		}


	}

	void Image::pre(Udjat::Abstract::Object &) {

	}

	void Image::post(Udjat::Abstract::Object &) {

	}

	void Image::write(Udjat::Dialog::Progress &progress) {

	}

 }
 #endif // HAVE_FATFS


