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

	const char * DataSource::PathFactory(const Udjat::XML::Node &node, const char *attrname) const {

		const char *path = String{node,attrname}.expand(node).expand().as_quark();
		debug(node.name(),"(",attrname,") = '",path,"'");

		if(!path[0]) {
			path = String{node,"url"}.expand(node).expand().as_quark();
			if(path[0]) {
				Logger::String{"Getting '",attrname,"' from 'url' attribute"}.trace(name());
			} else {
				Logger::String{"Attribute '",attrname,"' is missing"}.warning(name());
			}
		}

		return path;
	}

	const char * DataSource::path() const {
		const char *path = local();
		if(path[0] != '.') {
			throw logic_error("Unable to handle non relative path");
		}
		return path+1;
	}

	void DataSource::save(Udjat::Dialog::Progress &, const char *) {
		throw logic_error("Abstract datasource is unable to save to file");
	}

	std::string DataSource::save(const Udjat::Abstract::Object &, Udjat::Dialog::Progress &progress) {
		return save(progress);
	}

	std::string DataSource::save(Udjat::Dialog::Progress &) {
		throw logic_error("Abstract datasource is unable to save");
	}

	void DataSource::load(const Udjat::XML::Node &node, vector<std::shared_ptr<DataSource>> &sources) {
		for(Udjat::XML::Node nd = node; nd; nd = nd.parent()) {
			for(Udjat::XML::Node child = nd.child("source"); child; child = child.next_sibling("source")) {
				sources.push_back(make_shared<FileSource>(child));
			}
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

		const char * required_prefix = local();
		if(required_prefix[0] != '.') {
			throw logic_error("The source local path should be relative (starting with '.')");
		}

		if(repository.get() && repository->index()) {

			Logger::String{"Using indexed repository"}.trace(name());

			size_t szlocal = strlen(required_prefix);
			for(const auto &path : *repository) {

				if(strncmp(required_prefix,path.c_str(),szlocal)) {
					continue;
				}

				debug("path='",path.c_str(),"'");
				auto source = make_shared<FileSource>(path.c_str());
				source->rename(this->name());
				source->update_from_remote = this->update_from_remote;
				source->repository = this->repository;

				if(func(source)) {
					return true;
				}

			}

		}

		// TODO: Parse index.html

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

