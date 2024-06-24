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
 #include <udjat/tools/configuration.h>
 #include <stdexcept>

 #include <gtkmm.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	const char * Writer::devname = nullptr;
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

	void Writer::write(Udjat::Dialog::Progress &progress,const char *isoname) {
		open(progress);



		close();
	}

	void GtkWriter::open(Udjat::Dialog::Progress &progress) {

		class Dialog : public Gtk::Window {
		private:
			Gtk::Button cancel{_("_Cancel"),true}, apply{_("C_ontinue"),true};

		public:

			Dialog() {

				gtk_window_set_transient_for(
					GTK_WINDOW(gobj()),
					gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
				);

				set_modal(true);
				set_deletable(false);
				set_resizable(false);
				set_decorated(false);
				set_default_size(500,-1);
				set_title(_("Select destination device"));

				Gtk::Box view{Gtk::Orientation::VERTICAL};
				view.set_hexpand(true);
				view.set_vexpand(true);

				Gtk::Box contents{Gtk::Orientation::VERTICAL};
				contents.set_hexpand(true);
				contents.set_vexpand(true);
				contents.set_spacing(6);
				contents.set_margin(12);

				{
					Gtk::Label label;
					label.set_markup(
						Config::Value<string>{
							"messages",
							"insert-device-message",
							_("Insert an storage device <b>NOW</b> ")
						}.c_str()
					);
					label.get_style_context()->add_class("dialog-title");
					label.set_vexpand(false);
					contents.append(label);
				}

				{
					Gtk::Label label;
					label.set_markup(
						Config::Value<string>{
							"messages",
							"insert-device-body",
							_("This action will <b>DELETE ALL CONTENT</b> on the device.")
						}.c_str()
					);
					label.get_style_context()->add_class("dialog-subtitle");
					label.set_vexpand(false);
					contents.append(label);
				}
				view.append(contents);

				{
					Gtk::Box buttons{Gtk::Orientation::HORIZONTAL};
					buttons.set_hexpand(true);
					buttons.set_vexpand(false);
					buttons.set_homogeneous(true);

					cancel.set_hexpand(true);
					apply.set_hexpand(true);

					buttons.append(cancel);
					buttons.append(apply);

					view.append(buttons);
				}

				set_child(view);
			}

		};

		bool busy = true;

		Glib::signal_idle().connect([this,&busy](){

			Dialog *dialog = new Dialog();

			dialog->signal_destroy().connect([&busy]{
				busy = false;
			});

			dialog->present();

			return 0;

		});

		progress.hide();
		while(busy) {
			sleep(1);
		}
		progress.show();

		throw runtime_error("Incomplete");

	}

 }
