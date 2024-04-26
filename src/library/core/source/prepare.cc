/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <libreinstall/source.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/intl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <fcntl.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 using Progress = Udjat::Dialog::Progress;

 namespace Reinstall {

	/// @brief 'writer' for local files.
	class Local : public Source::File {
	private:
		const Udjat::File::Path from;

	public:
		Local(const Udjat::File::Path &f, const std::string &to) : Source::File{to}, from{f} {
		}

		bool remote() const noexcept override {
			return false;
		}

		const char * path() const noexcept override {
			return from.c_str();
		}

		void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {
			from.save(writer);
		}

	};

	// TODO: File source saving from url to 'path.local'. Will be used to recover local repository.

	/// @brief File source using URL to download to temp file.
	class Remote : public Source::File {
	private:
		std::string url;

	public:

		Remote(const std::string &u, const std::string &to) : Source::File{to}, url{u} {
		}

		bool remote() const noexcept override {
			return true;
		}

		const char * path() const override {
			throw runtime_error("Unable to get path for remote source");
		}

		void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {

			// Save URL to this->fd
			auto worker = Protocol::WorkerFactory(url.c_str());

			Logger::String{"Getting file from ",worker->url().c_str()}.write(Logger::Debug,"source");
			Progress::instance().url(worker->url().c_str());

			try {

				worker->save([&writer](unsigned long long current, unsigned long long total, const void *buf, size_t length){
					writer(current,total,buf,length);
					return true;
				});

			} catch(const std::exception &e) {

				Logger::String{"Error getting ",worker->url().c_str(),": ",e.what()}.error("source");
				throw;

			}

		}

	};

	void apache_mirror(const String &index, const char *base, const char *imgpath, Source::Files &files) {

		for(auto href = index.find("<a href=\""); href != string::npos; href = index.find("<a href=\"",href)) {

			auto from = href+9;
			href = index.find("\"",from);
			if(href == string::npos) {
				throw runtime_error(Logger::Message(_("Unable to parse file list from {}"),base));
			}

			string link = index.substr(from,href-from);

			if(link[0] =='/' || link[0] == '?' || link[0] == '.' || link[0] == '$')
				continue;

			if(link.size() >= 7 && strncmp(link.c_str(),"http://",7) == 0 ) {
				continue;
			}

			if(link.size() >= 8 && strncmp(link.c_str(),"https://",8) == 0 ) {
				continue;
			}

			string src{base};
			src += link;

			string dst{imgpath};
			if(dst.empty() || dst[dst.size()-1] != '/') {
				dst += '/';
			}
			dst += link;

			if(src[src.size()-1] == '/') {

				Progress &progress = Progress::instance();

				auto worker = Protocol::WorkerFactory(src.c_str());

				Logger::String{"Getting folder from ",worker->url().c_str()}.write(Logger::Debug,"apache");
				progress.url(worker->url().c_str());

				auto index = worker->get([&progress](double current, double total){
					progress.progress(current,total);
					return true;
				});

				if(index.empty()) {
					throw runtime_error(Logger::Message(_("Empty response from {}"),worker->url().c_str()));
				}

				apache_mirror(index, src.c_str(), dst.c_str(), files);

			} else {

				Logger::String{"Adding source file ",src}.write(Logger::Debug,"apache");
				files.insert(make_shared<Remote>(src,dst));

			}

		}

	}

	void Source::prepare(const Udjat::URL &local, const Udjat::URL &remote, Source::Files &files) const {

		Progress &progress = Progress::instance();

		if(!local.empty() && local.test()) {

			// Local path is available.
			Udjat::File::Path filepath{local.ComponentsFactory().path};
			filepath.realpath();

			Logger::String{"Getting file list from ",filepath.c_str()}.info(name());

			// Local files are available.
			progress.url(local.c_str());

			size_t szpath = filepath.size();
			filepath.for_each([this,szpath,&files](const Udjat::File::Path &path){

				const char *payload = path.c_str()+szpath;
				if(*payload == '/') {
					payload++;
				}

				string target{imgpath};
				target += payload;

				files.insert(make_shared<Local>(path,target));

				return false;

			},true);

			return;

		}

		// Get files from URL.
		if(remote[remote.size()-1] == '/') {

			// It's a remote folder, get contents.
			auto worker = Protocol::WorkerFactory(remote.c_str());
			worker->mimetype(MimeType::json);

			info("Getting file list for {}",worker->url().c_str());
			progress.url(worker->url().c_str());

			auto index = worker->get([&progress](double current, double total){
				progress.progress(current,total);
				return true;
			});

			if(index.empty()) {
				throw runtime_error(Logger::Message(_("Empty response from {}"),worker->url().c_str()));
			}

			/*
			trace(
				"Got a '{}' response from server '{}'",
					worker->response("Content-Type").value(),
					worker->response("Server").value()
			);
			*/

			// FIX-ME: Detect server.
			apache_mirror(index,worker->url().c_str(),imgpath,files);


		} else {

			// It's a remote file
			debug("src=",remote.c_str());
			debug("dst=",imgpath);

			Logger::String{"Adding source file ",remote.c_str()}.write(Logger::Debug,name());
			files.insert(make_shared<Remote>(remote,imgpath));

		}

	}


 }
