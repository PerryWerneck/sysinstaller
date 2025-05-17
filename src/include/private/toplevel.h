/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declare application window.
  */

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>

 class TopLevel : public Gtk::ApplicationWindow {
 private:

	class SideBar : public Gtk::Box {
	private:
		Gtk::Image logo;
   
	public:
	   SideBar() : Gtk::Box{Gtk::Orientation::VERTICAL} {
		get_style_context()->add_class("toplevel-sidebar");
		set_hexpand(false);
		set_vexpand(true);
	   }
   
	} sidebar;
   
 	class Label : public Gtk::Label {
	public:
	  // https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Label.html
	  Label(const char *style, const char *text) : Gtk::Label{text} {
		get_style_context()->add_class(style);
		set_wrap(true);
		set_halign(Gtk::Align::START);
		set_hexpand(true);
		set_vexpand(false);
	  }
	
	};
	
	class Button : public Gtk::Button {
	public:
	  Button(const char *style, const char *text) : Gtk::Button{text} {
		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Button.html
		get_style_context()->add_class(style);
		set_halign(Gtk::Align::END);
		get_style_context()->add_class("pill");
		set_hexpand(false);
		set_vexpand(false);
		set_use_underline(true);
	  }

	};

	Button apply{"suggested-action",_("_Apply")};
	Button cancel{"cancel-action",_("_Cancel")};

	Gtk::Box hbox{Gtk::Orientation::HORIZONTAL};
	Gtk::Box vbox{Gtk::Orientation::VERTICAL};

	Gtk::Box optionbox{Gtk::Orientation::VERTICAL};
	Gtk::Box buttons{Gtk::Orientation::HORIZONTAL};
	Gtk::ScrolledWindow viewport;

 public:
	TopLevel();
	~TopLevel() override;

 };
 