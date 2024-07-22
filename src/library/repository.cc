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
 #include <udjat/tools/configuration.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>
 #include <private/slpclient.h>
 #include <list>

 #ifdef HAVE_ZLIB
	#include <zlib.h>
 #endif // HAVE_ZLIB

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	std::shared_ptr<Repository> Repository::Factory(const Udjat::XML::Node &node) {

		static mutex guard;
		lock_guard<mutex> lock(guard);

		const char * name = XML::StringFactory(node,"repository","install");

		static list<std::shared_ptr<Repository>> repositories;

		for(auto repository : repositories) {
			if(!strcasecmp(repository->name(),name)) {
				return repository;
			}
		}

		for(auto parent = node;parent;parent = parent.parent()) {

			for(auto child = parent.child("repository");child;child = child.next_sibling("repository")) {

				if(strcasecmp(child.attribute("name").as_string(),name)) {
					continue;
				}

				if(!strcasecmp(XML::StringFactory(child,"repository",""),name)) {
					Logger::String{"Ignoring circular dependency"}.warning("repository");
					continue;
				}

				auto repo = make_shared<Repository>(child);
				repositories.push_back(repo);

				return repo;
			}

		}

		throw runtime_error(Logger::Message{_("Required repository '{}' not found"),name});

	}

	Repository::KParm::KParm(const Udjat::XML::Node &node) {
		for(auto child = node.child("attribute");child;child = child.next_sibling("attribute")) {
			if(!strcasecmp(child.attribute("name").as_string("none"),"kernel-parameter-name")) {
				name = String{child,"value"}.as_quark();
				slp = XML::QuarkFactory(node,"slp-value");
				enabled = child.attribute("allow-slp").as_bool(true);
			}
		}
	}

	Repository::Repository(const Udjat::XML::Node &node) : FileSource{node}, KernelParameter{node}, kparm{node}, slpclient{SLPClient::Factory(node)} {

		if(!(kparm.slp && *kparm.slp) && kparm.enabled) {

			// Build SLP value.

			const char *srvc = slpclient->service();
			if(!(srvc && *srvc)) {
				throw logic_error("SLP service name is not defined");
			}

			Logger::Message url{Config::Value<string>{"kernel-parameters","slp","slp://?{}&auto=1"}.c_str(),srvc};

			kparm.slp = url.as_quark();

		}

	}

	Repository::~Repository() {
	}

	bool Repository::index() {

		if(!files.empty()) {
			return true;
		}

#ifdef HAVE_ZLIB
		{

			// Try INDEX.gz

			auto &progress = Dialog::Progress::getInstance();

			URL url = url_remote();
			progress.url(url.c_str());
			url += "INDEX.gz";

			try {

				string filename;

				if(has_local()) {

					// Has local path, update file.

					filename = url_local().ComponentsFactory().path;
					File::Path::mkdir(filename.c_str());
					filename += "INDEX.gz";

					url.get(filename.c_str(),[&progress](double current, double total){
						progress = (total/current);
						return true;
					});

				} else {

					// No local path, use cache.

					filename = url.filename([&progress](double current, double total){
						progress = (total/current);
						return true;
					});

				}

				gzFile fd = gzopen(filename.c_str(), "r");
				if(!fd) {
					throw runtime_error("Error opening INDEX.gz");
				}

				char buffer[4096];
				memset(buffer,0,4096);
				while(gzgets(fd,buffer,4095)) {
					for(size_t ix = 0; ix < 4096 && buffer[ix]; ix++) {
						if(buffer[ix] < ' ') {
							buffer[ix] = 0;
						}
					}
					files.emplace_back(buffer);
				}

				gzclose(fd);

				Logger::String{"Got ",files.size()," filenames from repository index."}.trace(name());
				return true;

			} catch(const std::exception &e) {

				Logger::String{url.c_str(),": ",e.what()};

			}

			return true;

		}
#endif // HAVE_ZLIB

		// Parse html
		throw system_error(ENOTSUP, system_category(), _("Cant parse server file list"));

		return false;
	}

	const char * Repository::remote() const {

		const char *url = slpclient->url();
		if(url && *url) {
			return url;
		}

		return FileSource::remote();
	}

	std::string Repository::value(const Udjat::Abstract::Object &object) const {

		if(kparm.slp && *kparm.slp) {
			const char *url = slpclient->url();
			if(url && *url) {
				return kparm.slp;
			}
		}

		return remote();
	}

 }


