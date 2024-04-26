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
  * @brief Implements the abstract menu controller.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/ui/menu.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/intl.h>

 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Menu::Controller * Menu::Controller::cntrl = nullptr;

	Menu::Controller::Controller() {
		cntrl = this;
	}

	Menu::Controller::~Controller() {
		cntrl = nullptr;
	}

	Menu::Controller & Menu::Controller::instance() {
		if(!cntrl) {
			throw logic_error("Menu subsystem is not active");
		}
		return *cntrl;
	}

	void Menu::Controller::push_back(Item *item, const XML::Node &node) {

#if UDJAT_CHECK_VERSION(1,2,0)
		if(XML::AttributeFactory(node,"default").as_bool()) {
			def = item;
		}

		if(XML::AttributeFactory(node,"selected").as_bool()) {
			selected = item;
		}
#else
		if(XML::StringFactory(node,"default").as_bool()) {
			def = item;
		}

		if(XML::StringFactory(node,"selected").as_bool()) {
			selected = item;
		}
#endif

	}

	void Menu::Controller::remove(const Item *item) {
		if(selected == item) {
			selected = nullptr;
		}

		if(def == item) {
			def = nullptr;
		}
	}

	void Menu::Controller::set(Item *item, const Item::ActivationType type) noexcept {
		switch(type) {
		case Item::Selected:
			selected = item;
			break;

		case Item::Default:
			def = item;
			break;
		}
	}

	void Menu::Controller::activate(const Item::ActivationType type) {
		switch(type) {
		case Item::Selected:
			if(!selected) {
				throw logic_error(_("The menu is unselected"));
			}
			selected->activate();
			break;

		case Item::Default:
			if(!def) {
				throw runtime_error(_("No default option"));
			}
			def->activate();
			break;
		}
	}

 }
