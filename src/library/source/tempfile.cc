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

	TempFileSource::TempFileSource(const DataSource &src) : DataSource{src}, url{src.remote()}, filepath{src.path()} {
	}

	TempFileSource::TempFileSource(const char *n, const std::string &u, const std::string &p) : DataSource{n}, url{u}, filepath{p} {
	}

	TempFileSource::~TempFileSource() {
#ifndef DEBUG
		if(!filename.empty()) {
			unlink(filename.c_str());
		}
#endif // DEBUG
	}

	const char * TempFileSource::local() const {
		return "";
	}

	const char * TempFileSource::remote() const {
		return url.c_str();
	}

	const char * TempFileSource::path() const {
		return filepath.c_str();
	}

	std::string TempFileSource::save(const Udjat::Abstract::Object &object, Reinstall::Dialog::Progress &progress) {

		if(!filename.empty()) {
			return filename;
		}

		auto url = url_remote();

		debug("Downloading ",url.c_str());
		progress.url(url.c_str());

		try {

			filename = Udjat::File::Temporary::create();
			Udjat::File::Handler file{filename.c_str(),true};
			url.get([&progress,&file](uint64_t current, uint64_t total, const void *buf, size_t length){
				progress.file_sizes(current,total);
				file.write(buf,length);
				return false;
			});

		} catch(const std::exception &e) {

			Logger::String{url.c_str(),": ",e.what()}.error(name());
			throw;

		} catch(...) {

			Logger::String{url.c_str(),": Unexpected error"}.error(name());
			throw;

		}

		return filename;

	}

	void TempFileSource::save(Reinstall::Dialog::Progress &progress, const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer) {

		auto url = url_remote();
		progress = url.c_str();

		debug(url.c_str());

		throw runtime_error("TempFileSource::save Incomplete ");

	}

 }
