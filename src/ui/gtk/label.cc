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
  * @brief Implements GTK based Udjat::Label.
  */

 #include <config.h>

 #include <gtkmm.h>
 #include <glibmm/i18n.h>

 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>

 #include <udjat/ui/gtk/label.h>

 namespace Udjat {

	Label::Label(const Udjat::XML::Node &node, const char *attrname, const char *def)
#if UDJAT_CHECK_VERSION(1,2,0)
		: Label{XML::StringFactory(node,attrname,def)} {
#else
		: Label{XML::StringFactory(node,attrname,"value",def).c_str()} {
#endif

		if(!get_text().empty()) {
			show_all();
		}
	}

 }
