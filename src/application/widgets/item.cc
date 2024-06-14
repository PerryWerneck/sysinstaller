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

	auto icon = XML::AttributeFactory(node,"icon");
	if(icon) {

		debug("Using icon '",icon.as_string(),"'");
		margin = 1;

		Gtk::Image image;
		image.set_icon_size(Gtk::IconSize::INHERIT);
		image.get_style_context()->add_class("action-icon");
		image.set_from_icon_name(icon.as_string("image-missing"));

		grid.attach(image,0,0,1,2);
	}

	grid.attach(label,margin,0);
	grid.attach(body,margin,1);

	set_child(grid);

	get_style_context()->add_class("action-inactive");

	signal_toggled().connect([this]() {

		debug("Toggle!!");

		MainWindow & window{MainWindow::getInstance()};

		if(get_active()) {

			if(window.active && window.active != this) {
				window.active->set_active(false);
			}

			window.active = this;
			window.button.apply.set_sensitive(true);

			get_style_context()->remove_class("action-inactive");
			get_style_context()->add_class("action-active");
		} else {

			if(window.active == this) {
				window.active = nullptr;
				window.button.apply.set_sensitive(false);
			}

			get_style_context()->remove_class("action-active");
			get_style_context()->add_class("action-inactive");
		}

	});

	set_visible();

 }

 MainWindow::Item::~Item() {
 }
