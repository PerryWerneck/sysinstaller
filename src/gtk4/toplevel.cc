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
  * @brief Implements gtk4 toplevel.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <glib/gi18n.h>
 #include <private/toplevel.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <string>

 using namespace std;
 using namespace Udjat;


 TopLevel::TopLevel() : Gtk::ApplicationWindow() {
 
#ifdef DEBUG 
	get_style_context()->add_class("devel");
#endif


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
	set_default_size(800, 600);

	// Set up layout
	set_title(Config::Value<string>("toplevel","title",_("System reinstallation")));
	set_child(hbox);
	hbox.append(sidebar);
	hbox.append(vbox);

	{
		Gtk::Label title{_("Select an option")};
		title.get_style_context()->add_class("main-title");
		title.set_hexpand(true);
		title.set_vexpand(false);
		title.set_valign(Gtk::Align::START);
		vbox.append(title);
	}

	// Options
	{
		optionbox.set_hexpand(true);
		optionbox.set_vexpand(true);
		viewport.set_child(optionbox);


		viewport.get_style_context()->add_class("content-box");
		viewport.set_hexpand(true);
		viewport.set_vexpand(true);
		vbox.append(viewport);
	}

	// Button bar
	{
		buttons.set_hexpand(false);
		buttons.set_vexpand(false);
		buttons.set_valign(Gtk::Align::END);
		buttons.set_halign(Gtk::Align::END);
		buttons.set_homogeneous();	
		buttons.get_style_context()->add_class("button-box");
		buttons.set_spacing(3);
		buttons.append(cancel);
		buttons.append(apply);
		set_default_widget(apply);
		apply.set_sensitive(false);
		vbox.append(buttons);

		cancel.signal_clicked().connect([&]() {
			Gtk::Window::close();
		});
	
	}

	// Title
	/*
	*/

	/*



	*/

	present();
 }

 TopLevel::~TopLevel() {
 
 }
