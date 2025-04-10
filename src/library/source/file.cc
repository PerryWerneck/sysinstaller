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
 #include <reinstall/ui/progress.h>
 #include <udjat/tools/file.h>

 #include <unistd.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	FileSource::FileSource(const char *path) {
		url.remote = url.local = path;
	}

	FileSource::FileSource(const Udjat::XML::Node &node) : DataSource{node} {

		url.remote = PathFactory(node,"remote");
		url.local = PathFactory(node,"local");
		url.path = PathFactory(node,"destination",false);

		// Fixes loading of legacy control files.
		if(!(strcmp(url.local,url.remote) || strncmp(url.local,"http:",5))) {
			url.local = "";
		}

		if(url.remote[0] == '.' || url.local[0] == '.' || url.remote[0] == '/' || url.local[0] == '/') {
			repository = Repository::Factory(node);
		}

		if(!url.local[0] && url.remote[0] == '.') {
			url.local = url.remote;
		}

	}

	FileSource::~FileSource() {
	}

	bool FileSource::has_local() const noexcept {

		try {

			if(!(url.local && *url.local)) {
				return false;
			}

			if(url.local[0] != '.') {
				return true;
			}

			// Check if repository has local.
			if(repository && repository->has_local()) {
				return true;
			}

			if(url.local[strlen(url.local)-1] != '/') {
				Logger::String{"Unable to determine local path for ",url.remote}.trace(name());
			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(name());

		} catch(...) {

			Logger::String{"Unexpected error"}.error(name());

		}
		return false;
	}


	Udjat::XML::Node find(Udjat::XML::Node node, const char *nodename, bool required) {

		while(node) {
			if(!strcasecmp(node.name(),nodename)) {
				return node;
			}
			auto child = node.child(nodename);
			if(child) {
				return child;
			}
			node = node.parent();
		}

		if(required) {
			throw runtime_error(Logger::String{"Required node '",nodename,"' is missing"});
		}

		return Udjat::XML::Node{};

	}

	FileSource::FileSource(const Udjat::XML::Node &node, const char *nodename, bool required)
		: FileSource{find(node,nodename,required)} {
	}

	const char * FileSource::local() const {
		return url.local;
	}

	const char * FileSource::remote() const {
		return url.remote;
	}

	const char * FileSource::path() const {
		if(url.path && *url.path) {
			return url.path;
		}
		return DataSource::path();
	}


 }


