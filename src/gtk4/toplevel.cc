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
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/mainloop.h>
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

	// Title
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

	// TODO: Show loading popup.

	set_sensitive(false);
	ThreadPool::getInstance().push([this](){
		load_options();
		Glib::signal_idle().connect([this](){
			set_sensitive(true);
			present();
			return 0;
		});
	});

 }

 TopLevel::~TopLevel() {
 
 }

 TopLevel::SideBar::SideBar() : Gtk::Box{Gtk::Orientation::VERTICAL} {
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
	
 }

 // https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Label.html
 TopLevel::Label::Label(const char *style, const char *text) : Gtk::Label{text} {
	get_style_context()->add_class(style);
	set_wrap(true);
	set_halign(Gtk::Align::START);
	set_hexpand(true);
	set_vexpand(false);
 }

 TopLevel::Button::Button(const char *style, const char *text) : Gtk::Button{text} {
	// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Button.html
	get_style_context()->add_class(style);
	set_halign(Gtk::Align::END);
	get_style_context()->add_class("pill");
	set_hexpand(false);
	set_vexpand(false);
	set_use_underline(true);
  }

  std::shared_ptr<Reinstall::Group> TopLevel::group_factory(const Udjat::XML::Node &node) {
	
  }
