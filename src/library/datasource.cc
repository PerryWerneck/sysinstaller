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
  * @brief Implements Source repository..
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>

 using namespace Udjat;

 namespace Reinstall {

	DataSource::DataSource(const Udjat::XML::Node &node) : Udjat::NamedObject{node} {

		url.remote = XML::QuarkFactory(node,"remote",url.remote);
		if(!url.remote[0]) {
			url.remote = XML::QuarkFactory(node,"url",url.remote);
			if(url.remote[0]) {
				Logger::String{"Getting '",url.remote,"' from 'url' attribute"}.trace(name());
			} else {
				Logger::String{"No remote URL"}.warning(name());
			}
		}

		url.local = XML::QuarkFactory(node,"local");
		if(!url.local[0] && url.remote[0] == '.') {
			url.local = url.remote;
			Logger::String{"Using relative path '",url.local,"' for local url"}.trace(name());
		}

		if(url.local[0] == '.' || url.remote[0] == '.') {
			// Relative URLs, search for repository
			Logger::String{"Relative path, searching for repository '",XML::StringFactory(node,"repository","install"),"'"}.trace(name());
			repository = Repository::Factory(node);
		}

	}

	DataSource::~DataSource() {
	}

 }

