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
 #include <udjat/tools/file.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>
 #include <sys/stat.h>

 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	DataSource::DataSource(const Udjat::XML::Node &node) : Udjat::NamedObject{node} {

		url.remote = String{node,"remote"}.expand(node).expand().as_quark();
		if(!url.remote[0]) {
			url.remote = XML::QuarkFactory(node,"url",url.remote);
			if(url.remote[0]) {
				Logger::String{"Getting '",url.remote,"' from 'url' attribute"}.trace(name());
			} else {
				Logger::String{"No remote URL"}.warning(name());
			}
		}

		url.local = String{node,"local"}.expand(node).expand().as_quark();
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

	Udjat::URL DataSource::local() const {

		if(url.local[0] == '.') {
			if(!repository) {
				throw logic_error("Unable to use relative URLs without repository");
			}
			URL value{repository->local()};
			value += url.local;
			return value;
		}

		return URL{url.local};

	}

	Udjat::URL DataSource::remote() const {

		if(url.remote[0] == '.') {
			if(!repository) {
				throw logic_error("Unable to use relative URLs without repository");
			}
			URL value{repository->remote()};
			value += url.remote;
			return value;
		}

		return URL{url.remote};

	}

	void DataSource::save(Udjat::Dialog::Progress &progress, const char *path) {

		auto url = remote();

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

	std::string DataSource::save(Udjat::Dialog::Progress &progress) {

		auto url = local();
		if(!url.local()) {
			throw runtime_error("Unable to save to remote path");
		}

		auto components = url.ComponentsFactory();

		try {

			save(progress,components.path.c_str());

		} catch(...) {

			struct stat sb;
			const char *filename = components.path.c_str();
			if(stat(filename,&sb) != 0 || sb.st_blocks == 0 || (sb.st_mode & S_IFMT) != S_IFREG) {
				error() << "Download error, cached file '" << filename << "' not available" << endl;
				throw;
			}

			warning() << "Download error, using cached file '" << filename << "'" << endl;
		}

		return components.path;
	}

	void DataSource::load(const Udjat::XML::Node &node, vector<DataSource> &sources) {
		for(Udjat::XML::Node nd = node; nd; nd = nd.parent()) {
			for(Udjat::XML::Node child = nd.child("source"); child; child = child.next_sibling("source")) {
				sources.emplace_back(child);
			}
		}
	}


	bool DataSource::for_each(Udjat::Dialog::Progress &progress, const std::function<bool(const DataSource &value)> &func) const {

		if(repository.get() && repository->index()) {

			debug("Using repository index");

		}

			/*
		if(repository) {

			// Have repository, try 'INDEX.gz'
			URL url{repository->remote()};
			progress.url(url.c_str());
			url += "INDEX.gz";

			debug("Checking '",url.c_str(),"'");

			try {

				auto filename = url.filename([&progress](double current, double total){
					progress = (total/current);
					return true;
				});

				debug("filename='",filename.c_str(),"'");

			} catch(const std::exception &e) {

				Logger::String{url.c_str(),": ",e.what()};

			}

		}
			*/


		return false;
	}


 }

