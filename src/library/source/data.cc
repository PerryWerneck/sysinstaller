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
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/progress.h>

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
			throw logic_error(Logger::Message{"Unable to handle non relative path '{}'",path});
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

			// Has local path, using standard file source.
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
				source->message = this->message;
				source->rename(this->name());
				source->update_from_remote = this->update_from_remote;
				source->repository = this->repository;

				if(func(source)) {
					return true;
				}

			}

		} else {

			// No local files, using temporary file datasource.
			class TempFileSource : public DataSource {
			private:
				string filename;
				const std::string url;		///< @brief The URL for source in the remote server.
				const std::string filepath;	///< @brief Path for the file inside the destination image.

			public:

				TempFileSource(const char *n, const std::string &u, const std::string &p) : DataSource{n}, url{u}, filepath{p} {
				}

				~TempFileSource() {
#ifndef DEBUG
					if(!filename.empty()) {
						unlink(filename.c_str());
					}
#endif // DEBUG
				}

				const char * local() const override {
					return "";
				}

				const char * remote() const override {
					return url.c_str();
				}

				const char * path() const override {
					return filepath.c_str();
				}

				std::string save(const Udjat::Abstract::Object &object, Udjat::Dialog::Progress &progress) override {

					if(!filename.empty()) {
						return filename;
					}

					auto url = url_remote();
					progress = url.c_str();
					debug(url.c_str());

					filename = Udjat::File::Temporary::create();

					debug("Saving to ",filename.c_str());
					Udjat::File::Handler file{filename.c_str(),true};
					url.get([&progress,&file](uint64_t current, uint64_t total, const void *buf, size_t length){
						progress.file_sizes(current,total);
						file.write(buf,length);
						return true;
					});

					return filename;

				}

				void save(Udjat::Dialog::Progress &progress, const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer) {

					auto url = url_remote();
					progress = url.c_str();

					debug(url.c_str());

					throw runtime_error("TempFileSource::save Incomplete ");

				}


			};

			// Get reference file.
			const char *prefix = path();
			if(prefix[0] != '.') {
				prefix = remote();
			}
			if(prefix[0] != '.') {
				throw logic_error("Cant expand non relative repository");
			}

			size_t szprefix = strlen(prefix);
			for(const auto &path : *repository) {

				if(strncmp(prefix,path.c_str(),szprefix)) {
					continue;
				}

				auto source = make_shared<TempFileSource>(name(),path,path);
				source->message = this->message;
				source->repository = this->repository;

				if(func(source)) {
					return true;
				}

			}

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

