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
  * @brief Implements GTK4 group widget.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>

 #include <gtkmm.h>
 #include <private/mainwindow.h>

 #include <memory>

 using namespace Udjat;
 using namespace std;

 MainWindow::Group::Group(const XML::Node &node) :
	Reinstall::Group{node},
	title{ (*this)["title"], Gtk::Align::START },
	sub_title{ XML::AttributeFactory(node,"sub-title").as_string(), Gtk::Align::START } {

	debug("Building group '",title.get_text().c_str(),"'");

	// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Grid.html

	set_hexpand(true);
	set_halign(Gtk::Align::FILL);

	set_vexpand(false);
	set_valign(Gtk::Align::START);

	contents.set_hexpand(true);
	contents.set_halign(Gtk::Align::FILL);

	contents.set_vexpand(false);
	contents.set_valign(Gtk::Align::START);

	get_style_context()->add_class("group-box");

	title.get_style_context()->add_class("group-title");
	sub_title.get_style_context()->add_class("group-subtitle");

	int margin = 0;

	auto icon = XML::AttributeFactory(node,"icon");
	if(icon) {
		debug("Using icon '",icon.as_string(),"'");
		margin = 1;

		Gtk::Image image;
		image.set_icon_size(Gtk::IconSize::LARGE);
		image.get_style_context()->add_class("group-icon");
		image.set_from_icon_name(icon.as_string("image-missing"));

		attach(image,0,0,1,2);
	}

	attach(title,margin,0);
	attach(sub_title,margin,1);
	attach(contents,margin,2);

	set_visible();

 }

 void MainWindow::Group::push_back(const Udjat::XML::Node &node, std::shared_ptr<Udjat::Abstract::Object> child) {

	debug("-------------------> ",child->name());

	auto item = make_shared<MainWindow::Item>(node,child);

	MainWindow::getInstance().itens.push_back(item);
	contents.append(*item);
	item->set_visible();

	Reinstall::Group::push_back(node,child);
 }
