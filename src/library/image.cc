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
  * @brief Implements abstract image.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <functional>
 #include <udjat/tools/url.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/disk/abstract.h>
 #include <udjat/ui/progress.h>

 using namespace Udjat;

 namespace Reinstall {

 	Abstract::Image::~Image() {
 	}

	void Abstract::Image::append(DataSource &source) {

		std::string from = source.save(Dialog::Progress::getInstance());
		append(from.c_str(),source.image_path());

	}

 }

