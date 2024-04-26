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
  * @brief Implement the application menu item.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <gtkmm.h>
 #include <glibmm/i18n.h>
 #include <private/mainwindow.h>
 #include <string.h>
 #include <udjat/ui/gtk/label.h>

 using namespace Udjat;
 using namespace ::Gtk;
 using namespace std;

 // http://transit.iut2.upmf-grenoble.fr/doc/gtkmm-3.0/reference/html/classGtk_1_1RadioButton.html
 MainWindow::Item::Item(Menu::Item *m, const Udjat::XML::Node &node) :
	menu{m},
	title{node,"title"},
	subtitle{node,"sub-title"},
	icon{node,::Gtk::ICON_SIZE_DND,"icon-name"} {

	static ::Gtk::RadioButton::Group group;

	set_group(group);

	set_hexpand(true);
	set_vexpand(false);
	set_valign(ALIGN_START);
	set_halign(ALIGN_FILL);

	set_inconsistent();
	set_mode(false);

	get_style_context()->add_class("action-button");
	grid.get_style_context()->add_class("action-container");
	title.get_style_context()->add_class("action-title");

	grid.attach(title,1,0,2,1);

	if(subtitle) {
		subtitle.get_style_context()->add_class("action-subtitle");
		grid.attach(subtitle,1,1,2,1);
	}

#if UDJAT_CHECK_VERSION(1,2,0)
	if(XML::AttributeFactory(node,"show-icon").as_bool(true)) {
		icon.get_style_context()->add_class("action-icon");
		grid.attach(icon,0,0,1,2);
	}
#else
	if(XML::StringFactory(node,"show-icon","value","true").as_bool(true)) {
		icon.get_style_context()->add_class("action-icon");
		grid.attach(icon,0,0,1,2);
	}
#endif

	add(grid);

	help.get_style_context()->add_class("action-help");

	show_all();

 }

 void MainWindow::push_back(Menu::Item *menu, const XML::Node &node) {

	Menu::Controller::push_back(menu,node);

	auto item = make_shared<MainWindow::Item>(menu,node);
	auto group = find(node,"group");
	bool active = node.attribute("default").as_bool(false);

	items.push_back(item);

	item->set_name((group->get_name() + "-" + XML::StringFactory(node,"name")).c_str());

	// The group URL
	{
		const char *url = node.attribute("help-url").as_string();

		if(url && *url) {

			item->help.set_uri(url);
			item->help.set_label(url);

		} else {

			for(pugi::xml_node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

				if(strcasecmp("help",child.attribute("name").as_string("")) == 0) {

					url = child.attribute("url").as_string();
					if(!(url && *url)) {
						url = child.attribute("value").as_string();
					}

					debug(url);

					if(url && *url) {
						item->help.set_uri(url);
						item->help.set_label(child.attribute("label").as_string(url));

						const char *tooltip = child.attribute("tooltip").as_string();
						if(tooltip && *tooltip) {
							item->help.set_tooltip_text(tooltip);
						}
					}

					break;
				}
			}
		}

	}

	Glib::signal_idle().connect([this,item,group,active](){

		item->signal_toggled().connect([this,item](){
			if(item->get_active()) {
				Logger::String{"Selecting '",item->get_name().c_str(),"' (",item->get_text(),")"}.trace("mainwindow");
				this->selected = item;
				buttons.apply.set_sensitive();
			}
		});

		group->push_back(*item);
		if(active) {
			item->set_active(true);
		}
		item->toggled();

		if(!item->help.get_uri().empty()) {
			item->help.set_hexpand(false);
			item->help.set_vexpand(false);
			item->help.set_valign(::Gtk::ALIGN_START);
			item->help.set_halign(::Gtk::ALIGN_END);
			item->help.show_all();
			group->push_back(item->help);
		}

		return 0;

	});

 }
