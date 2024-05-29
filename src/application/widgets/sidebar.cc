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
  * @brief Implements sidebar widget.
  */

 #include <config.h>

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <gtkmm.h>
 #include <widgets/sidebar.h>
 #include <string>

 // https://gtkmm.org/en/documentation.html

 using namespace std;
 using namespace Udjat;

 SideBar::SideBar() : Gtk::Box{Gtk::Orientation::VERTICAL}{

	get_style_context()->add_class("sidebar");

	set_hexpand(false);
	set_vexpand(true);
	set_homogeneous(false);

	logo.get_style_context()->add_class("sidebar_logo");

#ifdef DEBUG
	logo.property_file().set_value(Config::Value<string>{"MainWindow","logo","./icon.svg"}.c_str());
#else
	logo.property_file().set_value(Config::Value<string>{"MainWindow","logo",Application::DataFile("icon.svg").c_str()}.c_str());
#endif // DEBUG

	append(logo);

//	show_all();
 }

 SideBar::~SideBar() {
 }

 void SideBar::set(const Udjat::XML::Node &node) {
 }

