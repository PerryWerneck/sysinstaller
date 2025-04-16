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
 #include <reinstall/ui/progress.h>
 #include <udjat/tools/configuration.h>
 #include <stdexcept>
 #include <semaphore.h>

 #include <fcntl.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 #include <gtkmm.h>
 #include <private/gtkremovabledevicedialog.h>

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


	Writer::Writer() {
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

		auto progress = Udjat::Dialog::Progress::getInstance();
		progress->set(this->url.c_str());

		try {

			struct stat st;
			if(fstat(fd,&st) != 0) {
				throw std::system_error(errno,std::system_category(),"Cant get file stats");
			}

			size(st.st_size);

			progress->hide();
			open();
			progress->show();

			unsigned long long offset = 0;
			uint8_t buffer[st.st_blksize];
			while(offset < ((unsigned long long) st.st_size)) {

				ssize_t bytes = read(fd,buffer,st.st_blksize);
				if(bytes < 0) {
					throw std::system_error(errno,std::system_category(),"Error reading source file");
				} else if(bytes == 0) {
					throw runtime_error("Unexpected EOF on source file");
				}

				debug(offset,"/",st.st_size);

				progress->set((uint64_t) offset,(uint64_t) st.st_size);
				Writer::write(offset,buffer,(unsigned long long) bytes);
				offset += bytes;

			}
			progress->set((uint64_t) offset,(uint64_t) st.st_size);

		} catch(...) {

			close();

			throw;
		}

		progress->set(_("Finishing..."));
		close();

	}

	void GtkWriter::open() {

		if(!selected.empty()) {

			// Use pre-selected output.
			try {

				Writer::open(selected.c_str());

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error("writer");
				close();

			}

			this->url = selected.c_str();
			Logger::String{"Writing image to ",this->url.c_str()}.info("writer");
	
			return;

		}

		struct {
			string devdescr;
			string devname;
			int response = -1;
			sem_t semaphore;
		} info;

		sem_init(&info.semaphore,0,0);

		while(!*this) {

			Glib::signal_idle().connect([this,&info](){

				auto *dialog = new GtkRemovableDeviceDialog(*this,settings);

#ifdef USE_MESSAGE_DIALOG
				dialog->signal_response().connect([dialog,&info](int rsp){
					info.response = rsp;
					info.devdescr = dialog->description();
					info.devname = dialog->device();
					Logger::String{"User selected '",info.devdescr,"' (",info.response,")"}.trace("writer");
					sem_post(&info.semaphore);
					delete dialog;
				});
#else
				#error Needs implementation
#endif // USE_MESSAGE_DIALOG

				dialog->present();

				return 0;

			});

			sem_wait(&info.semaphore);

			if(info.response) {
				Logger::String{"Device selection dialog exits with rc=",info.response," (",strerror(info.response),")"}.warning("writer");
				close();
				throw runtime_error(strerror(info.response));
			}

			if(!*this) {

				try {

					debug("Opening ",info.devname.c_str());
					Writer::open(info.devname.c_str());

				} catch(const std::exception &e) {

					Logger::String{e.what()}.error("writer");
					close();

				}

			}

		}

		this->url = info.devdescr.c_str();
		Logger::String{"Writing image to ",this->url.c_str()}.info("writer");
	
	}

 }
