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
  * @brief Implements GTK based Udjat::Icon.
  */

 #include <config.h>

 #include <gtkmm.h>
 #include <glibmm/i18n.h>

 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>

 #include <udjat/ui/gtk/icon.h>

 #include <string>

 using namespace std;

 namespace Udjat {

	Gtk::Icon::Icon(const char *name, const ::Gtk::IconSize iconsize, bool symbolic) {

		// https://developer-old.gnome.org/gtkmm/stable/classGtk_1_1Image.html

		if(name && *name && strcasecmp(name,"none")) {

			if(symbolic) {
					set_from_icon_name((std::string{name} + "-symbolic").c_str(), iconsize);
			} else {
					set_from_icon_name(name, iconsize);
			}

			show();

		} else {

			hide();

		}

	}

	Gtk::Icon::Icon(const Udjat::XML::Node &node, const ::Gtk::IconSize iconsize, const char *attrname, const char *def) :
#if UDJAT_CHECK_VERSION(1,2,0)
		Icon{ XML::StringFactory(node,attrname,def), iconsize, true } {
#else
		Icon{ XML::StringFactory(node,attrname,"value",def).c_str(), iconsize, true } {
#endif
	}

 }
