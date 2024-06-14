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
  * @brief Implements abstract action.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/action.h>
 #include <reinstall/group.h>

 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Action::Action(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node) : NamedObject{node} {

		debug("Building action '",name(),"'");

		auto *group = dynamic_cast<const Group *>(&parent);

		if(!group) {
			throw logic_error("Action parent should be a group controller");
		}

	}

	Action::~Action() {
		debug("Destroying action '",name(),"'");
	}

	void Action::activate() {
		throw runtime_error("Invalid action");
	}

	bool Action::initialize() {
		return true;
	}

 }

