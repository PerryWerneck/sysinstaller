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
  * @brief Implements Source repository.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/progress.h>
 #include <udjat/tools/file.h>

 #include <reinstall/tools/datasource.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	FileSource::FileSource(const Udjat::XML::Node &node) : DataSource{node} {

		url.remote = PathFactory(node,"remote");
		url.local = PathFactory(node,"local");

		if(!url.local[0] && url.remote[0] == '.') {
			url.local = url.remote;
			Logger::String{"Using relative path '",url.local,"' for local files"}.trace(name());
		} else {
			Logger::String{"Using '",url.local,"' for local files"}.trace(name());
		}

		if(url.remote[0]) {
			Logger::String{"Using '",url.remote,"' for remote files"}.trace(name());
		}

	}

	const char * FileSource::local() const {
		return url.local;
	}

	const char * FileSource::remote() const {
		return url.local;
	}

 }


