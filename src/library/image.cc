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
 #include <reinstall/tools/writer.h>
 #include <reinstall/image.h>
 #include <udjat/ui/progress.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

 	Abstract::Image::~Image() {
 	}

	void Abstract::Image::append(std::shared_ptr<DataSource> source) {
		append(source->save(Dialog::Progress::getInstance()).c_str(),source->path());
	}

	void Abstract::Image::write(Udjat::Dialog::Progress &dialog, const std::function<void(unsigned long long offset, const void *contents, unsigned long long length)> &task) {
		throw runtime_error(_("No write support on selected image"));
	}

	void Abstract::Image::write(Udjat::Dialog::Progress &progress, const Udjat::Dialog &settings) {

		progress = _("Writing image");
		Reinstall::Writer &writer = Reinstall::Writer::getInstance();
		writer.open(progress,settings);

		write(progress,[&writer](unsigned long long offset, const void *contents, unsigned long long length){
			writer.write(offset,contents,length);
		});
	}

	void Abstract::Image::append(Udjat::Dialog::Progress &progress, list<std::shared_ptr<DataSource>> &sources) {

		size_t item = 0;
		for(auto &source : sources) {
			progress.item(++item,sources.size());
			append(source);
		}
		progress.item();

	}

 }

