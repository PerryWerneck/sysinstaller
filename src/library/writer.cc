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
 #include <stdexcept>
 #include <semaphore.h>

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

	void Writer::write(Udjat::Dialog::Progress &progress, const Udjat::Dialog &dialog, const char *isoname) {
		open(progress,dialog);



		close();
	}

	void GtkWriter::open(Udjat::Dialog::Progress &progress, const Udjat::Dialog &settings) {

		struct {
			string devdescr;
			string devname;
			int response = -1;
			sem_t semaphore;
		} info;

		sem_init(&info.semaphore,0,0);

		progress.hide();
		progress.file_sizes(0,0);

		while(!*this) {

			Glib::signal_idle().connect([this,&info,&settings](){

				auto *dialog = new GtkRemovableDeviceDialog(*this,settings);

#ifdef USE_MESSAGE_DIALOG
				dialog->signal_response().connect([dialog,&info](int rsp){
					info.response = rsp;
					info.devdescr = dialog->description();
					info.devname = dialog->device();
					debug("Response=",info.response," (",info.devdescr,")");
					sem_post(&info.semaphore);
					delete dialog;
				});
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

					Writer::open(info.devname.c_str());

				} catch(const std::exception &e) {

					Logger::String{e.what()}.error("writer");
					close();

				}

			}

		}

		progress.url(info.devdescr.c_str());
		progress.show();

		throw runtime_error("Incomplete");

	}

 }
