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
 MainWindow::MainWindow(Glib::RefPtr<::Gtk::Application> app) : Gtk::ApplicationWindow{app}, Udjat::Factory{"group",moduleinfo} {

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1ApplicationWindow.html

		/*
        {
                auto css = Gtk::CssProvider::create();
#ifdef DEBUG
                css->load_from_path("./stylesheet.css");
#else
                css->load_from_path(Application::DataFile("stylesheet.css").c_str());
#endif // DEBUG
//                get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        */

        set_title(Config::Value<string>("MainWindow","title",_("System reinstallation")));
        set_default_size(800, 600);

        set_icon_name(Config::Value<string>{"MainWindow","icon",PRODUCT_ID "." PACKAGE_NAME}.c_str());

        layout.box.append(layout.sidebar);
//        layout.swindow.append(layout.view);
        layout.box.append(layout.swindow);

//        layout.box.show_all();
        set_child(layout.box);
 }

 MainWindow::~MainWindow() {
 }

 bool MainWindow::generic(const pugi::xml_node &node) {
	return false;
 }
