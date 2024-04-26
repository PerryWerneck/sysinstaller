/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implement the application class.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>

 #include <udjat/module.h>
 #include <udjat/factory.h>

 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>

 #include <private/application.h>
 #include <private/mainwindow.h>

 #include <libreinstall/writer.h>

 using namespace std;

 static const struct {
        char to;
        const char *from;
        const char *help;
 } options[] = {
        { 'O',  "output",   		"\t\tOutput to file"		},
        { 'L',  "output-length",	"\tLength of output file"	},
 };

 static const Udjat::ModuleInfo moduleinfo{PACKAGE_NAME " application"};

 /// @brief Factory for window properties.
 class MainWindow::PropertyFactory : public Udjat::Factory {
 private:
	MainWindow &hwnd;

 public:
	PropertyFactory(MainWindow &window) : Udjat::Factory{"mainwindow",moduleinfo}, hwnd{window} {
	}

	bool generic(const Udjat::XML::Node &node) override {

#if UDJAT_CHECK_VERSION(1,2,0)

		hwnd.set_title(
			Udjat::XML::StringFactory(
				node,
				"title",
				hwnd.get_title().c_str()
			)
		);

		hwnd.layout.title.set_text(
			Udjat::XML::StringFactory(
				node,
				"sub-title",
				hwnd.layout.title.get_text().c_str()
			)
		);

		hwnd.buttons.cancel.set_label(
			Udjat::XML::StringFactory(
				node,
				"cancel",
				hwnd.buttons.cancel.get_label().c_str()
			)
		);

		hwnd.buttons.apply.set_label(
			Udjat::XML::StringFactory(
				node,
				"apply",
				hwnd.buttons.apply.get_label().c_str()
			)
		);

		hwnd.logo.set(
			Udjat::XML::StringFactory(
				node,
				"logo",
				"logo"
			)
		);

#else

		hwnd.set_title(
			Udjat::XML::StringFactory(
				node,
				"title",
				"value",
				hwnd.get_title().c_str()
			)
		);

		hwnd.layout.title.set_text(
			Udjat::XML::StringFactory(
				node,
				"sub-title",
				"value",
				hwnd.layout.title.get_text().c_str()
			)
		);

		hwnd.buttons.cancel.set_label(
			Udjat::XML::StringFactory(
				node,
				"cancel",
				"value",
				hwnd.buttons.cancel.get_label().c_str()
			)
		);

		hwnd.buttons.apply.set_label(
			Udjat::XML::StringFactory(
				node,
				"apply",
				"value",
				hwnd.buttons.apply.get_label().c_str()
			)
		);

		hwnd.logo.set(
			Udjat::XML::StringFactory(
				node,
				"logo",
				"value",
				"logo"
			).c_str()
		);

#endif // UDJAT_CHECK_VERSION


		return true;
	}

 };


 Glib::RefPtr<Application> Application::create() {

#ifdef DEBUG
	Udjat::Logger::enable(Udjat::Logger::Trace);
	Udjat::Logger::enable(Udjat::Logger::Debug);
	Udjat::Logger::console(true);
#endif // DEBUG

	// Create application
	auto application = Glib::RefPtr<Application>{new Application(PRODUCT_ID "." PACKAGE_NAME)};
	set_default(application);

	application->window = new MainWindow();

	return application;

 }

 void Application::on_startup() {
	Udjat::Gtk::Application::on_startup();
	add_window(*window);
 }

 void Application::on_activate() {
	Udjat::Gtk::Application::on_activate();
	window->show_all();
 }

 int Application::init(const char *definitions) {

	MainWindow::PropertyFactory wpfactory{*window};

	/// @brief Factory for <group> nodes.
	class GroupFactory : public Udjat::Factory {
	private:
		MainWindow &controller;

	public:
		GroupFactory(MainWindow &window) : Udjat::Factory{"group",moduleinfo}, controller{window} {
		}

		bool generic(const Udjat::XML::Node &node) override {

			// Findo group.
			controller.find(node,"name");

			// Load group children.
			for(auto child : node) {

				Factory::for_each(child.name(),[this,&child](Factory &factory) {

					try {

						return factory.generic(child);

					} catch(const std::exception &e) {

						factory.error() << "Cant parse node <" << child.name() << ">: " << e.what() << std::endl;

					} catch(...) {

						factory.error() << "Cant parse node <" << child.name() << ">: Unexpected error" << std::endl;

					}

					return false;

				});

			}

			return true;
		}

	} gfactory{*window};

#ifdef DEBUG
	Udjat::Module::load(Udjat::File::Path(".bin/Debug/modules"));
	definitions = "./xml.d";
#endif // DEBUG

	return Udjat::Gtk::Application::init(definitions);

 }

 void Application::help(std::ostream &out) const noexcept {

	super::help(out);
	for(auto &option : options) {
		out << "  --" << option.from << option.help << endl;
	}

 }

 bool Application::argument(const char *name, const char *value) {

	for(auto &option : options) {
		if(!strcasecmp(name,option.from)) {
			return argument(option.to,value);
		}
	}

 	return super::argument(name,value);
 }

 bool Application::argument(const char name, const char *value) {

	switch(name) {
	case 'L':
		Reinstall::Writer::set_device_length(Udjat::String{value}.as_ull());
		break;

	case 'O':
		if(!value) {
			throw runtime_error("Output file name is requires");
		}
		Reinstall::Writer::set_device_name(value);
		break;

	default:
		return super::argument(name,value);
	}

	return true;
 }

