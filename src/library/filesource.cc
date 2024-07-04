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

		if(url.remote[0] == '.' || url.local[0] == '.') {
			repository = Repository::Factory(node);
		}

		if(!url.local[0] && url.remote[0] == '.') {
			url.local = url.remote;
		}

		if(Logger::enabled(Logger::Trace)) {
			if(url.local[0]) {
				Logger::String{"Using '",url_local().c_str(),"' for local files"}.trace(name());
			}
			if(url.remote[0]) {
				Logger::String{"Using '",url_remote().c_str(),"' for remote files"}.trace(name());
			}
		}

	}

	const char * FileSource::local() const {
		return url.local;
	}

	const char * FileSource::remote() const {
		return url.remote;
	}

	Udjat::URL FileSource::url_local() const {

		const char *path = local();

		if(path[0] == '.') {
			if(!repository) {
				throw logic_error("Unable to use relative URLs without repository");
			}

			URL url{repository->local()};
			url += path;

			return url;
		}

		return URL{path};

	}

	Udjat::URL FileSource::url_remote() const {

		const char *path = remote();

		if(path[0] == '.') {
			if(!repository) {
				throw logic_error("Unable to use relative URLs without repository");
			}

			URL url{repository->remote()};
			url += path;

			return url;
		}

		return URL{path};

	}

	void FileSource::save(Udjat::Dialog::Progress &progress, const char *path) {

		auto url = url_remote();

		info() << "Downloading " << url.c_str() << endl;

		debug("Downloading '",url.c_str(),"' to '",path,"'");

		{
			string str{path};
			auto pos = str.rfind('/');
			if(pos == string::npos) {
				throw runtime_error("Invalid local path");
			}
			str.resize(pos);
			if(File::Path::mkdir(str.c_str())) {
				info() << "New path made: " << str << endl;
			}
		}

		try {

			progress.url(url.c_str());
			url.get(path,[&](uint64_t current, uint64_t total){

				progress.file_sizes(current,total);

				return true;
			});

		} catch(const std::exception &e) {

			error() << url.c_str() << " -> " << path << ": " << e.what() << endl;
			throw;
		}

	}

	std::string FileSource::save(Udjat::Dialog::Progress &progress) {

		auto url = url_local();
		auto components = url.ComponentsFactory();

		if(!update_from_remote && access(components.path.c_str(),R_OK) == 0) {
			Logger::String{components.path.c_str()," already exists"}.write(Logger::Debug,name());
			return components.path.c_str();
		}

		const char *filename = components.path.c_str();

		try {

			save(progress,filename);

		} catch(...) {

			struct stat sb;
			if(stat(filename,&sb) != 0 || sb.st_blocks == 0 || (sb.st_mode & S_IFMT) != S_IFREG) {
				error() << "Download error, cached file '" << filename << "' not available" << endl;
				throw;
			}

			warning() << "Download error, using cached file '" << filename << "'" << endl;
		}

		return components.path;

	}

 }


