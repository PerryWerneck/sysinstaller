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
  * @brief Implement GTK4 version of writer::open.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/tools/writer.h>
 
 #include <semaphore.h>
 #include <gtkmm.h>
 #include <private/gtkremovabledevicedialog.h>

 #include <private/toplevel.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	void TopLevel::open(const Reinstall::Dialog &settings) {

		if(!Writer::selected.empty()) {

			// Use pre-selected output.
			try {

				Writer::open(Writer::selected.c_str());

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error("writer");
				Writer::close();
				throw;

			}

			this->device_url = Writer::selected.c_str();
			Logger::String{"Writing image to ",this->device_url.c_str()}.info("writer");
	
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

			Glib::signal_idle().connect([this,&info,&settings](){

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
				Writer::close();
				throw runtime_error(strerror(info.response));
			}

			if(!*this) {

				try {

					debug("Opening ",info.devname.c_str());
					Writer::open(info.devname.c_str());

				} catch(const std::exception &e) {

					Logger::String{e.what()}.error("writer");
					Writer::close();
					throw;

				}

			}

		}

		this->device_url = info.devdescr.c_str();
		Logger::String{"Writing image to ",this->device_url.c_str()}.info("writer");
	
	}

 }
