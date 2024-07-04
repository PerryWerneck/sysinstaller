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

	DataSource::DataSource(const Udjat::XML::Node &node) : Udjat::NamedObject{node} {

#ifdef DEBUG
		update_from_remote = XML::AttributeFactory(node,"update-from-remote").as_bool(false);
#else
		update_from_remote = XML::AttributeFactory(node,"update-from-remote").as_bool(update_from_remote);
#endif // DEBUG

		/*

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

		url.image = String{node,"image-path"}.expand(node).expand().as_quark();
		if(!url.image[0]) {
			if(url.local[0] == '.') {
				url.image = url.local;
				Logger::String{"Using relative path '",url.image,"' for image path"}.trace(name());
			} else if(url.remote[0] == '.') {
				url.image = url.remote;
				Logger::String{"Using relative path '",url.image,"' for image path"}.trace(name());
			}
		}
		*/

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

	/*
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
	*/

	/*
	const char * DataSource::image_path() const {
		if(url.local[0] != '.') {
			throw logic_error("Unable to handle non relative path");
		}
		return url.local+1;
	}
	*/

	/*
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
	*/

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

	bool DataSource::for_each(Udjat::Dialog::Progress &progress, std::vector<std::shared_ptr<Template>> &templates, const std::function<bool(std::shared_ptr<DataSource> value)> &func) const {

		/*
		if(!this->url.local || this->url.local[0] != '.') {
			throw logic_error("A local relative path is required");
		}

		if(repository.get() && repository->index()) {

			debug("Using repository index");

			size_t szlocal = strlen(this->url.local);

			for(const auto &path : *repository) {

				if(strncmp(this->url.local,path.c_str(),szlocal)) {
					continue;
				}

				debug("TODO: ",path.c_str());

				// URL Based data source
				auto source = make_shared<DataSource>();
				source->rename(this->name());
				source->update_from_remote = this->update_from_remote;
				source->repository = this->repository;
				source->url.local = source->url.remote = source->url.image = path.c_str();

				// Check templates.
				for(auto &tmplt : templates) {
					if(tmplt == source->url.remote) {
						Logger::String{"Using template '",tmplt.name(),"' for ",path.c_str()}.trace(name());

						break;
					}
				}


				if(func(source)) {
					return true;
				}

			}

		}

		// TODO: Parse index.html

		*/
		return false;
	}


 }

