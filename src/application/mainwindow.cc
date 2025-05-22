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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/threadpool.h>
 #include <reinstall/ui/progress.h>
 #include <udjat/tools/application.h>
 #include <string>

 #include <gtkmm.h>
 #include <private/mainwindow.h>

 using namespace Udjat;
 using namespace std;

 MainWindow * MainWindow::instance = nullptr;

 MainWindow::Button::Button()
 	: apply{_("_Apply"), true}, cancel{_("_Cancel"), true}, box{Gtk::Orientation::HORIZONTAL,6} {
 }

 MainWindow::Layout::Layout()
	: vbox{Gtk::Orientation::VERTICAL}, title{ _( "Select option" ), Gtk::Align::START }, contents{Gtk::Orientation::VERTICAL} {
 }	
 
 MainWindow::MainWindow(Glib::RefPtr<::Gtk::Application> app) : Gtk::ApplicationWindow{app} {

	instance = this;

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

	{
		Config::Value<string> icon{"MainWindow","icon",G_STRINGIFY(PACKAGE_DOMAIN)};
		Logger::String{"Setting toplevel icon to '",icon.c_str(),"'"}.info();
		set_icon_name(icon.c_str());
	}

	layout.box.set_hexpand(true);
	layout.box.set_vexpand(true);

	layout.vbox.get_style_context()->add_class("contents");

	// A wide variety of style classes may be applied to labels, such as .title, .subtitle, .dim-label, etc
	layout.title.get_style_context()->add_class("main-title");
	layout.title.set_vexpand(false);
	layout.vbox.append(layout.title);

	layout.contents.get_style_context()->add_class("contents");
	layout.contents.set_hexpand(true);
	layout.contents.set_vexpand(true);
	layout.contents.set_valign(Gtk::Align::START);
	layout.contents.set_halign(Gtk::Align::FILL);

	layout.swindow.set_hexpand(true);
	layout.swindow.set_vexpand(true);
	layout.swindow.get_style_context()->add_class("content-box");

	layout.swindow.set_child(layout.contents);

	layout.vbox.append(layout.swindow);

	layout.box.append(layout.sidebar);

	button.box.get_style_context()->add_class("button-box");
	button.box.set_homogeneous();
	button.box.append(button.cancel);
	button.box.append(button.apply);

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
		Gtk::Window::close();
	});

	button.apply.signal_clicked().connect([&]() {

		if(*selected) {

			button.apply.set_sensitive(false);
			button.cancel.set_sensitive(false);
			layout.vbox.set_sensitive(false);

			if(selected) {
				selected->activate();
			}

			button.apply.set_sensitive(true);
			button.cancel.set_sensitive(true);
			layout.vbox.set_sensitive(true);

		}

	});

 }

 MainWindow::~MainWindow() {
 	instance = nullptr;
 	itens.clear();
 }

 std::shared_ptr<Reinstall::Group> MainWindow::get(const Udjat::XML::Node &node) {

 	// FIX-ME: Search group by name before building a new one.

	auto grp = make_shared<Group>(node);
	layout.contents.append(*grp);

	return grp;
 }




