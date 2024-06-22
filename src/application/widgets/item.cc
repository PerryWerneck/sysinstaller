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
  * @brief Implements window item.
  */

 // Reference: https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1CheckButton.html

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/url.h>
 #include <string>

 #include <gtkmm.h>
 #include <private/mainwindow.h>

 using namespace Udjat;
 using namespace std;

 MainWindow::Item::Item(const Udjat::XML::Node &node, std::shared_ptr<Udjat::Abstract::Object> a)
	: action{a}, label{ XML::AttributeFactory(node,"title").as_string(), Gtk::Align::START }, body{ XML::AttributeFactory(node,"sub-title").as_string(), Gtk::Align::START } {

	set_hexpand(true);
	set_vexpand(false);
	set_valign(Gtk::Align::START);
	set_halign(Gtk::Align::FILL);

	get_style_context()->add_class("action-button");
	grid.get_style_context()->add_class("action-container");
	label.get_style_context()->add_class("action-title");
	body.get_style_context()->add_class("action-subtitle");

	int margin = 0;

	Reinstall::Action *act = dynamic_cast<Reinstall::Action *>(action.get());

	const char *icon_name;
	if(act) {
		icon_name = act->icon_name();
	} else {
		icon_name = XML::AttributeFactory(node,"icon-name").as_string();
	}

	if(icon_name && *icon_name) {

		debug("Using icon '",icon_name,"'");
		margin = 1;

		Gtk::Image image;
		image.set_icon_size(Gtk::IconSize::LARGE);
		image.get_style_context()->add_class("action-icon");
		image.set_from_icon_name(icon_name);

		grid.attach(image,0,0,1,2);
	}

	grid.attach(label,margin,0);
	grid.attach(body,margin,1);

	set_child(grid);

	get_style_context()->add_class("action-inactive");

	signal_toggled().connect([this]() {

		MainWindow & window{MainWindow::getInstance()};

		if(get_active()) {

			if(window.selected && window.selected != this) {
				window.selected->set_active(false);
			}

			window.selected = this;
			window.button.apply.set_sensitive(true);

			get_style_context()->remove_class("action-inactive");
			get_style_context()->add_class("action-active");

		} else {

			if(window.selected == this) {
				window.selected = nullptr;
				window.button.apply.set_sensitive(false);
			}

			get_style_context()->remove_class("action-active");
			get_style_context()->add_class("action-inactive");
		}

	});

	set_visible();
	set_sensitive(false);

	auto action = dynamic_cast<Reinstall::Action *>(a.get());

	if(action) {
		ThreadPool::getInstance().push([this,action](){
			try {
				if(action->initialize()) {
					action->info() << "Initialization complete, enabling item" << endl;
					Glib::signal_idle().connect([this](){
						set_sensitive(true);
						return 0;
					});
				} else {
					action->warning() << "Initialization has failed, item will be disabled" << endl;
				}
			} catch(const exception &e) {
				action->error() << e.what() << endl;
			}
		});
	}

 }

 MainWindow::Item::~Item() {
 }

 void MainWindow::Item::activate() const {

	auto action = dynamic_cast<Reinstall::Action *>(this->action.get());
	if(action) {
		action->activate();
	}


 }
