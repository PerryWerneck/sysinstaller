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
 #include <udjat/ui/progress.h>
 #include <udjat/ui/gtk4/application.h>
 #include <udjat/tools/threadpool.h>

 #include <memory>
 #include <gtkmm.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Dialog::Progress> Gtk::Application::ProgressFactory() {

		class Progress : public Udjat::Dialog::Progress, private ::Gtk::Window {
		private:

		public:
			Progress() {
			}

			~Progress() {
			}

			int run(const std::function<int(Dialog::Progress &progress)> &task) noexcept override {

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

		return make_shared<Progress>();

	}


 }

