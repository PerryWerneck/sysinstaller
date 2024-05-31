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
  * @brief Implements main window.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/factory.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/module/info.h>
 #include <string>

 #include <gtkmm.h>
 #include <private/mainwindow.h>

 using namespace Udjat;
 using namespace std;

 static const Udjat::ModuleInfo moduleinfo{"Reinstall"};
 MainWindow::MainWindow(Glib::RefPtr<::Gtk::Application> app) : Gtk::ApplicationWindow{app}, Udjat::Factory{PACKAGE_NAME,moduleinfo} {

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1ApplicationWindow.html

        {
                auto css = Gtk::CssProvider::create();
#ifdef DEBUG
                css->load_from_path("./stylesheet.css");
#else
                css->load_from_path(Application::DataFile("stylesheet.css").c_str());
#endif // DEBUG
                get_style_context()->add_provider_for_display(Gdk::Display::get_default(),css,GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }

        set_deletable(false);

        set_title(Config::Value<string>("MainWindow","title",_("System reinstallation")));
        set_default_size(800, 600);

        set_icon_name(Config::Value<string>{"MainWindow","icon",PRODUCT_ID "." PACKAGE_NAME}.c_str());

        layout.box.set_hexpand(true);
        layout.box.set_vexpand(true);

        layout.vbox.get_style_context()->add_class("contents");

        // A wide variety of style classes may be applied to labels, such as .title, .subtitle, .dim-label, etc
        layout.title.get_style_context()->add_class("main-title");
        layout.title.set_vexpand(false);
        layout.vbox.append(layout.title);

        layout.view.get_style_context()->add_class("main-view");
        layout.view.set_hexpand(true);
        layout.view.set_vexpand(true);
        layout.view.set_valign(Gtk::Align::START);
        layout.view.set_halign(Gtk::Align::START);

        layout.swindow.set_hexpand(true);
        layout.swindow.set_vexpand(true);

        layout.swindow.set_child(layout.view);

        layout.vbox.append(layout.swindow);

        layout.box.append(layout.sidebar);

		button.box.get_style_context()->add_class("buttons");
		button.box.set_homogeneous();
        button.box.append(button.apply);
        button.box.append(button.cancel);

        button.apply.set_sensitive(false);

		button.box.set_valign(Gtk::Align::END);
		button.box.set_halign(Gtk::Align::END);
		button.box.set_hexpand(false);
        button.box.set_vexpand(false);
        layout.vbox.append(button.box);

        layout.vbox.set_hexpand(true);
        layout.vbox.set_vexpand(true);
        layout.box.append(layout.vbox);

        set_child(layout.box);

		button.cancel.signal_clicked().connect([&]() {
			close();
		});

 }

 MainWindow::~MainWindow() {
 }

 static int TypeFactory(const char *name) {

	static const char * types[] = {
		"MainWindow",
		"group",
	};

	for(int id = 0; id < (int) (sizeof(types)/sizeof(types[0]));id++) {

		if(!strcasecmp(name,types[id])) {
			return id;
		}

	}

	return -1;
 }

 int MainWindow::compare(const char *name) const noexcept {

	if(TypeFactory(name) >= 0) {
		debug("Found ",name);
		return 0;
	}

	return Udjat::Factory::compare(name);

 }


 bool MainWindow::NodeFactory(const XML::Node &node) {

	int type{TypeFactory(node.name())};

	if(type < 0) {
		return false;
	}

	// Build node
	switch(type) {
	case 0:	// MainWindow
		{
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
				{
					"label",
					[](MainWindow &window, const XML::Node &node) {
						window.layout.title.set_markup(node.attribute("value").as_string());
					}
				},


			};

			for(auto &property : properties) {
				for(auto child = node.child("attribute");child;child = child.next_sibling("attribute")) {
					if(!strcasecmp(child.attribute("name").as_string("none"),property.name)) {
						property.apply(*this,child);
					}
				}
			}

		}
		break;

	default:
		Logger::String{"Unexpected configuration type '",node.name(),"'"}.warning("MainWindow");
	}


	return true;
 }
