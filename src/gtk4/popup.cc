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
  * @brief Implements gtk4 popup.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/ui/popup.h>
 #include <udjat/ui/gtk4/application.h>
 #include <udjat/tools/threadpool.h>

 #include <memory>
 #include <gtkmm.h>
 #include <gtkmm/alertdialog.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Dialog::Popup> Gtk::Application::PopupFactory() {

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1AlertDialog.html

		class Popup : public Udjat::Dialog::Popup, private ::Gtk::AlertDialog {
		public:
			Popup() {
				gtk_window_set_transient_for(
					GTK_WINDOW(gobj()),
					gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
				);
				set_modal(true);
			}

			~Popup() {
			}

			void message(const char *message) override {
				string str{message};
				Glib::signal_idle().connect([this,str](){
					set_message(str);
					return 0;
				});
			}

			void detail(const char *text) override {
				string str{text};
				Glib::signal_idle().connect([this,str](){
					set_detail(str);
					return 0;
				});
			}

			int run(const std::function<int(Dialog::Popup &popup)> &task) noexcept override {

				int rc = -1;
				auto mainloop = Glib::MainLoop::create();

				Udjat::ThreadPool::getInstance().push([this,&task,&rc,mainloop](){

					try {

						rc = task(*this);

					} catch(const std::exception &e) {

						// TODO: Show error popup

						rc = -1;
						Logger::String{e.what()}.error("gtk");

					} catch(...) {

						// TODO: Show error popup

						rc = -1;
						Logger::String{"Unexpected error running background task"}.error("gtk");

					}

					Glib::signal_idle().connect([mainloop](){
						mainloop->quit();
						return 0;
					});

				});

				mainloop->run();

				return rc;

			}

		};

		return make_shared<Popup>();

	}


 }

