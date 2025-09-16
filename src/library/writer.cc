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
  * @brief Implement device writer.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/tools/writer.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/progress.h>
 #include <udjat/tools/configuration.h>
 #include <reinstall/dialog.h>
 #include <stdexcept>
 #include <semaphore.h>

 #include <fcntl.h>
 #include <sys/stat.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	std::string Writer::selected;
	Writer * Writer::instance = nullptr;

	Writer & Writer::getInstance() {
		if(instance) {
			return *instance;
		}
		throw runtime_error(_("The device writer is not available"));
	}

	Writer::Writer(const char *name) : writer_name{name} {
		if(instance) {
			throw logic_error(_("Writer is already available"));
		}
		instance = this;
	}

	Writer::~Writer() {
		if(instance == this) {
			instance = nullptr;
		}
	}

	void Writer::write(const char *isoname) {

		int fd = ::open(isoname,O_RDONLY);
		if(fd < 0) {
			throw std::system_error(errno,std::system_category(),"Cant open source file");
		}

		try {

			write(fd);

		} catch(...) {

			::close(fd);
			throw;

		}

		::close(fd);

	}

	void Writer::write(int fd) {

		try {

			struct stat st;
			if(fstat(fd,&st) != 0) {
				throw std::system_error(errno,std::system_category(),"Cant get file stats");
			}

			size(st.st_size);

			{
				Reinstall::Dialog dummy;
				open(dummy);
			}
			
			auto progress = Udjat::Dialog::Progress::getInstance();

			Logger::String{"Writing image to ",device_url.c_str()}.info();
			progress->url(device_url.strip().c_str());

			unsigned long long offset = 0;
			uint8_t buffer[st.st_blksize];
			while(offset < ((unsigned long long) st.st_size)) {

				ssize_t bytes = read(fd,buffer,st.st_blksize);
				if(bytes < 0) {
					throw std::system_error(errno,std::system_category(),"Error reading source file");
				} else if(bytes == 0) {
					throw runtime_error("Unexpected EOF on source file");
				}

				// debug(offset,"/",st.st_size);

				progress->set((uint64_t) offset,(uint64_t) st.st_size);
				Writer::write(offset,buffer,(unsigned long long) bytes);
				offset += bytes;

			}
			progress->set((uint64_t) offset,(uint64_t) st.st_size);
			progress->url(_("Finishing..."));
			progress->done();


		} catch(...) {

			close();

			throw;
		}

		close();

	}

 }
