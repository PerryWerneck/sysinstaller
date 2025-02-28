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
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <gtkmm.h>
 #include <widgets/sidebar.h>
 #include <string>

 // https://gtkmm.org/en/documentation.html

 using namespace std;
 using namespace Udjat;

 SideBar::SideBar() : Gtk::Box{Gtk::Orientation::VERTICAL}{

	get_style_context()->add_class("toplevel-sidebar");

	set_hexpand(false);
	set_vexpand(true);
	set_homogeneous(false);

#ifdef DEBUG
	Config::Value<string> path{"defaults","sidebar-logo","./icons/logo.svg"};
#else
	Config::Value<string> path{"defaults","sidebar-logo",Application::DataFile("icons/logo.svg").c_str()};
#endif // DEBUG

	try {

		Logger::String{"Getting logo from '",path.c_str(),"'"}.trace("sidebar");

		logo.set_pixel_size(128);
		logo.set(Gdk::Pixbuf::create_from_file(path.c_str()));
		logo.get_style_context()->add_class("toplevel-sidebar-logo");
		append(logo);

	} catch(const std::exception &e) {

		Logger::String{path.c_str(),": ",e.what()}.error("sidebar");

	}


	set_visible();

 }

 SideBar::~SideBar() {
 }

 void SideBar::set(const Udjat::XML::Node &) {
 }

