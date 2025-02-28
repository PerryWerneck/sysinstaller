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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/ui/gtk4/application.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/tools/factory.h>
 #include <reinstall/tools/writer.h>
 #include <reinstall/tools/kernelparameter.h>
 #include <reinstall/tools/repository.h>

 #include <private/mainwindow.h>

 #include <reinstall/modules/isowriter.h>
 #include <reinstall/modules/isobuilder.h>

 #include <udjat/module/http.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char* argv[]) {

	static const Udjat::ModuleInfo moduleinfo{"Reinstall"};

 	class Application : public Udjat::Gtk::Application, private Udjat::Factory {
	private:
		MainWindow *window = nullptr;

	public:
		Application() :  Udjat::Factory{"MainWindow",moduleinfo} {
		}

		~Application() {
			if(window) {
				delete window;
				window = nullptr;
			}
		}

		void help(std::ostream &out) const noexcept override {
			Udjat::Gtk::Application::help(out);
			cout	<< "  --output=img\tWrite resulting image to file 'img' instead of usb" << endl
					<< "  --kparm=n=v\tSet kernel parameter 'n' to 'v' on boot image" << endl
					<< "  --repo=r=u\tSet url for repository 'r' to 'u', disable slp" << endl
					<< "  --quit\tNon interactive exit" << endl
					<< "  --reboot\tNon interactive reboot" << endl;
		}

		bool argument(const char *name, const char *value) override {

			if(value && (strcasecmp(name,"usb-output-device") == 0 || strcasecmp(name,"output") == 0)) {
				Reinstall::Writer::set_output(value);
				return true;
			}

			if(value && (strcasecmp(name,"kernel-parameter") == 0 || strcasecmp(name,"kparm") == 0)) {
				Reinstall::KernelParameter::preset(value);
				return true;
			}

			if(value && (strcasecmp(name,"repository") == 0 || strcasecmp(name,"repo") == 0)) {
				Reinstall::Repository::preset(value);
				return true;
			}

			if(strcasecmp(name,"quit") == 0) {
				Udjat::Dialog::set_default(Udjat::Dialog::NonInteractiveQuit);
				return true;
			}

			if(strcasecmp(name,"reboot") == 0) {
				Udjat::Dialog::set_default(Udjat::Dialog::NonInteractiveReboot);
				return true;
			}

			if(strcasecmp(name,"non-interactive") == 0) {
				Udjat::Dialog::set_default(Udjat::Dialog::NonInteractive);
				return true;
			}

			return Udjat::Gtk::Application::argument(name,value);
		}

		bool argument(const char name, const char *value) {

			if(value && name == 'O') {
				Reinstall::Writer::set_output(value);
				return true;
			}

			if(value && name == 'K') {
				Reinstall::KernelParameter::preset(value);
				return true;
			}

			return Udjat::Gtk::Application::argument(name,value);
		}

		bool NodeFactory(const Udjat::XML::Node &node) override {

			if(!window) {
				throw logic_error("Cant parse node before main window");
			}

			// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Window.html

			static const struct {
				const char *name;
				const std::function<void(MainWindow &window, const XML::Node &node)> apply;
			} properties[] = {
				{
					"title",
					[](MainWindow &window, const XML::Node &node) {
						window.set_title(node.attribute("value").as_string());
					}
				},
				{
					"modal",
					[](MainWindow &window, const XML::Node &node) {
						window.set_modal(node.attribute("value").as_bool());
					}
				},
				{
					"icon",
					[](MainWindow &window, const XML::Node &node) {
						window.set_icon_name(node.attribute("value").as_string(PACKAGE_NAME));
					}
				},
				{
					"resizable",
					[](MainWindow &window, const XML::Node &node) {
						window.set_resizable(node.attribute("value").as_bool());
					}
				},
				/*
				{
					"label",
					[](MainWindow &window, const XML::Node &node) {
						window.layout.title.set_markup(node.attribute("value").as_string());
					}
				},
				*/


			};

			for(auto &property : properties) {
				for(auto child = node.child("attribute");child;child = child.next_sibling("attribute")) {
					if(!strcasecmp(child.attribute("name").as_string("none"),property.name)) {
						property.apply(*window,child);
					}
				}
			}

			return true;


		}

		void startup(Glib::RefPtr<::Gtk::Application> app, const char *definitions) override {
			Udjat::Application::info() << "Building main window" << endl;
			window = new MainWindow(app);
			debug("Definitions='",definitions,"'");

			{
				Application::LibDir path{"modules"};
#ifdef DEBUG
				path.assign(".bin/Debug/");
#endif // DEBUG
				Logger::String{"Loading application modules from '",path.c_str(),"'"}.trace(Udjat::Application::name());

				Udjat::HTTP::Module::Factory();
				Reinstall::IsoWriter::Module::Factory();
				Reinstall::IsoBuilder::Module::Factory();

				Udjat::Module::load(path,false);

			}

			super::startup(app,definitions);
		}

		void activate(Glib::RefPtr<::Gtk::Application> app, const char *definitions) override {
			debug("Definitions='",definitions,"'");
			super::activate(app,definitions);
			if(window) {
				window->present();
			} else {
				Udjat::Application::error() << "No window on activate signal" << endl;
			}
		}

		void shutdown(Glib::RefPtr<::Gtk::Application> app, const char *definitions) override {
			if(window) {
				window->hide();
			} else {
				Udjat::Application::error() << "No window on shutdown signal" << endl;
			}
			debug("Definitions='",definitions,"'");
			super::shutdown(app,definitions);
		}

 	};

#ifdef DEBUG
	Config::allow_user_homedir(true);
	Logger::verbosity(9);
	Logger::redirect();
	Logger::console(true);
	return Application{}.run(argc,argv,"./xml.d");
#else
	Logger::redirect();
	return Application{}.run(argc,argv);
#endif // DEBUG


 }

