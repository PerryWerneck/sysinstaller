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
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>

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

	Repository::Repository(const Udjat::XML::Node &node) : FileSource{node}, KernelParameter{node}, kparm{node}, slpclient{SLPClient::Factory(node)} {

		if(!(kparm.slp && *kparm.slp) && kparm.enabled) {

			// Build SLP value.

			const char *srvc = slpclient->service();
			if(!(srvc && *srvc)) {
				throw logic_error("SLP service name is not defined");
			}

			if(!(kparm.slp && *kparm.slp)) {
				Logger::Message url{Config::Value<string>{"kernel-parameters","slp","slp://?{}&auto=1"}.c_str(),srvc};
				kparm.slp = url.as_quark();
			}

		}

	}

	Repository::~Repository() {
	}

	bool Repository::operator==(const Repository &repo) const noexcept {

		if(strcasecmp(name(),repo.name())) {
			return false;
		}

		if(strcasecmp(url.remote,repo.url.remote)) {
			return false;
		}

		if(slpclient.get() && repo.slpclient.get()) {
			return *slpclient == *repo.slpclient;
		}

		return true;
	}

	bool Repository::index(const char *filename) {
#ifdef HAVE_ZLIB
		gzFile fd = gzopen(filename, "r");
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
#else
		return false;
#endif // HAVE_ZLIB
	}

	bool Repository::index() {

		if(!files.empty()) {
			return true;
		}

#ifdef HAVE_ZLIB
		{
			debug("Trying index.gz");

			// Try INDEX.gz

			auto &progress = Dialog::Progress::getInstance();

			URL url = url_remote();

			progress.url(url.c_str());
			url += "INDEX.gz";

			Logger::String{"Searching for ",url.c_str()}.trace(name());

			try {

				string filename;

				if(has_local()) {

					// Has local path, update file.
					debug("Using local file");

					filename = url_local().path();
					File::Path::mkdir(filename.c_str());
					filename += "INDEX.gz";

					debug("------------->",filename.c_str());

					url.get(filename.c_str(),[&progress](double current, double total){
						progress = (total/current);
						return false;
					});

					return index(filename.c_str());

				} else {

					// No local path, use cache.
					debug("Using remote file");

					filename = url.tempfile([&progress](double current, double total){
						progress = (total/current);
						return true;
					});

					bool rc = index(filename.c_str());

					unlink(filename.c_str());

					return rc;
				}


			} catch(const std::exception &e) {

				Logger::String{url.c_str(),": ",e.what()}.error(name());
				return false;

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

		String value;

		if(kparm.slp && *kparm.slp) {
			const char *url = slpclient->url();
			if(url && *url) {
				value = kparm.slp;
			}
		}

		if(value.empty()) {
			value = remote();
		}

		value.expand(object);

		if(value.empty()) {
			throw logic_error(Logger::Message{_("Kernel parameter for repository '{}' has an empty value"),name()});
		}

		return value;
	}

	void Repository::preset(const char *arg) {
		const char *ptr = strchr(arg,'=');

		if(!ptr) {
			ptr = strchr(arg,':');
		}

		if(!ptr) {
			throw runtime_error("Invalid repository parameter definition");
		}

		preset(string{arg,(size_t) (ptr-arg)}.c_str(),ptr+1);
	}

 }


