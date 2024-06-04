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
  * @brief Application entry point.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/logger.h>
 #include <udjat/ui/gtk4/application.h>

 #include <private/mainwindow.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char* argv[]) {

 	class Application : public Udjat::Gtk::Application {
	private:
		MainWindow *window = nullptr;

	public:
		~Application() {
			if(window) {
				delete window;
				window = nullptr;
			}
		}

		void startup(Glib::RefPtr<::Gtk::Application> app, const char *definitions) override {
			info() << "Building main window" << endl;
			window = new MainWindow(app);
			debug("Definitions='",definitions,"'");
#ifdef DEBUG
			Udjat::Module::load(Udjat::File::Path{".bin/Debug/isowriter.so"});
#endif // DEBUG
			super::startup(app,definitions);
		}

		void activate(Glib::RefPtr<::Gtk::Application> app, const char *definitions) override {
			debug("Definitions='",definitions,"'");
			super::activate(app,definitions);
			if(window) {
				window->present();
			} else {
				error() << "No window on activate signal" << endl;
			}
		}

		void shutdown(Glib::RefPtr<::Gtk::Application> app, const char *definitions) override {
			if(window) {
				window->hide();
			} else {
				error() << "No window on shutdown signal" << endl;
			}
			debug("Definitions='",definitions,"'");
			super::shutdown(app,definitions);
		}

 	};

#ifdef DEBUG
	Logger::verbosity(9);
	Logger::redirect();
	Logger::console(true);
	return Application{}.run(argc,argv,"./xml.d");
#else
	Logger::redirect();
	return Application{}.run(argc,argv);
#endif // DEBUG


 }

