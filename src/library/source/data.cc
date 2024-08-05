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
 #include <udjat/tools/file.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>
 #include <reinstall/tools/template.h>
 #include <sys/stat.h>

 #include <stdexcept>
 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	DataSource::DataSource(const DataSource &src)
		: Udjat::NamedObject{src.name()}, repository{src.repository}, update_from_remote{src.update_from_remote} {
		this->message = src.message;
	}

	DataSource::DataSource(const Udjat::XML::Node &node) : Udjat::NamedObject{node} {

#ifdef DEBUG
		update_from_remote = XML::AttributeFactory(node,"update-from-remote").as_bool(false);
#else
		update_from_remote = XML::AttributeFactory(node,"update-from-remote").as_bool(update_from_remote);
#endif // DEBUG

	}

	DataSource::~DataSource() {
	}

	const char * DataSource::PathFactory(const Udjat::XML::Node &node, const char *attrname, bool required) const {

		String value{node,attrname};
		value.expand(node).expand();
		if(value.empty()) {
			value = String{node,"url"}.expand(node).expand();
		}

		if(value.empty()) {
			if(required) {
				throw runtime_error(Logger::String{"Required attribute '",attrname,"' is missing or invalid"});
			}
			return "";
		}

		if(value[0] != '/') {
			return value.as_quark();
		}

		if(node.attribute("repository")) {
			String relative{"."};
			relative += value;
			return relative.as_quark();
		}

		return value.as_quark();
	}

	const char * DataSource::path() const {
		const char *path = local();
		if(path[0] != '.') {
			throw logic_error("Unable to handle non relative path");
		}
		return path+1;
	}

	bool DataSource::has_local() const noexcept {

		try {

			const char *ptr = local();

			return (ptr && *ptr);

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(name());

		} catch(...) {

			Logger::String{"Unexpected error checking local path"}.error(name());

		}

		return false;
	}

	void DataSource::save(Udjat::Dialog::Progress &progress, const char *path) {

		auto url = url_remote();

		info() << "Downloading " << url.c_str() << endl;

		if(message && *message) {
			progress = message;
		}

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

	std::string DataSource::save(const Udjat::Abstract::Object &object, Udjat::Dialog::Progress &progress) {

		if(has_local()) {

			auto url = url_local();
			url.expand(object);

			auto components = url.ComponentsFactory();

			if(!update_from_remote && access(components.path.c_str(),R_OK) == 0) {
				Logger::String{components.path.c_str()," already exists"}.write(Logger::Debug,name());
				return components.path.c_str();
			}

			const char *filename = components.path.c_str();

			try {

				DataSource::save(progress,filename);

			} catch(...) {

				struct stat sb;
				if(stat(filename,&sb) != 0 || sb.st_blocks == 0 || (sb.st_mode & S_IFMT) != S_IFREG) {
					error() << "Download error, cached file '" << filename << "' not available" << endl;
					throw;
				}

				warning() << "Download error, using cached file '" << filename << "'" << endl;
			}

			return components.path;

		} else {

			throw runtime_error("Incomplete: DataSource::save()");

		}

	}

	std::string DataSource::save(Udjat::Dialog::Progress &progress) {
		return save(Udjat::Abstract::Object{},progress);
	}

	void DataSource::save(Udjat::Dialog::Progress &progress, const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer) {

		const char *local_filename = this->local();

		if(local_filename && *local_filename) {

			// Has local (cache) file, try to use it.
			Udjat::File::Handler{save(progress).c_str()}.save(writer);
			return;
		}

		// No cache, download it directly.
		auto url = url_remote();

		url.get(writer);

	}

	void DataSource::load(const Udjat::XML::Node &node, vector<std::shared_ptr<DataSource>> &sources, const char *nodename) {

		if(nodename) {
			for(Udjat::XML::Node nd = node; nd; nd = nd.parent()) {
				for(Udjat::XML::Node child = nd.child(nodename); child; child = child.next_sibling(nodename)) {
					sources.push_back(make_shared<FileSource>(child));
				}
			}
		} else {
			load(node,sources,"source");
			load(node,sources,"driver-installation-disk");
		}

	}

	bool DataSource::for_each(const std::function<bool(const char *filename)> &func) const {

		if(repository.get() && repository->index()) {
			for(const std::string &filename : *repository) {
				if(func(filename.c_str())) {
					return true;
				}
			}
		}

		return false;
	}

	bool DataSource::for_each(Udjat::Dialog::Progress &progress, const std::function<bool(std::shared_ptr<DataSource> value)> &func) const {

		if(message && *message) {
			progress = message;
		}

		if(!(repository.get() && repository->index())) {
			throw runtime_error(_("Invalid repository"));
		}

		if(has_local()) {

			std::string required_prefix{local()};

			// Setup local path.
			if(required_prefix[0] != '.') {
				Logger::Message message{"Invalid local path: {}, should start with '.'",required_prefix.c_str()};
				if(Config::Value{"application","legacy",true}) {
					const char *ptr = local();
					if(ptr[0] == '/') {
						required_prefix = ".";
						required_prefix += ptr;
						message.warning(name());
					}
				} else {
					throw logic_error(message);
				}
			}

			size_t szlocal = required_prefix.size();
			for(const auto &path : *repository) {

				if(strncmp(required_prefix.c_str(),path.c_str(),szlocal)) {
					continue;
				}

				auto source = make_shared<FileSource>(path.c_str());
				source->rename(this->name());
				source->update_from_remote = this->update_from_remote;
				source->repository = this->repository;

				if(func(source)) {
					return true;
				}

			}

		} else {

			throw runtime_error("Incomplete");

		}

		/*
		std::string required_prefix{local()};
		if(required_prefix[0] != '.') {
			Logger::Message message{"Invalid local path: {}, should start with '.'",required_prefix.c_str()};
			if(Config::Value{"application","legacy",true}) {
				const char *ptr = local();
				if(ptr[0] == '/') {
					required_prefix = ".";
					required_prefix += ptr;
					message.warning(name());
				}
			} else {
				throw logic_error(message);
			}
		}

		if(repository.get() && repository->index()) {

			Logger::String{"Using indexed repository"}.trace(name());

			size_t szlocal = required_prefix.size();
			for(const auto &path : *repository) {

				if(strncmp(required_prefix.c_str(),path.c_str(),szlocal)) {
					continue;
				}

				//debug("path='",path.c_str(),"'");
				auto source = make_shared<FileSource>(path.c_str());
				source->rename(this->name());
				source->update_from_remote = this->update_from_remote;
				source->repository = this->repository;

				if(func(source)) {
					return true;
				}

			}

		}
		*/

		return false;

	}

	Udjat::URL DataSource::url_local() const {

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

	Udjat::URL DataSource::url_remote() const {

		const char *path = remote();

		if(path[0] == '.' || path[0] == '/') {

			if(!repository) {
				throw logic_error("Unable to use relative URLs without repository");
			}

			URL url{repository->remote()};
			url += path;

			return url;
		}

		return URL{path};

	}

 }

