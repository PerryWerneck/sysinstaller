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
  * @brief Implement group.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/group.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/factory.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	//
	// Group controller
	//
	static const Udjat::ModuleInfo moduleinfo{"libreinstall"};

	Group::Controller * Group::Controller::instance = nullptr;

	Group::Controller::Controller() : Udjat::Factory{"group", moduleinfo} {
		if(instance) {
			throw system_error(EBUSY,system_category(),_("Group controller is already active"));
		}
		instance = this;
	}

	Group::Controller::~Controller() {
		instance = nullptr;
	}

	Group::Controller & Group::Controller::getInstance() {
		if(instance) {
			return *instance;
		}
		throw logic_error(_("No active group controller"));
	}

	bool Group::Controller::NodeFactory(const Udjat::XML::Node &node) {
		auto group = get(node);
		if(!group) {
			return false;
		}
		group->setup(node);
		return true;
	}

	//
	// Group
	//
	Group::Group(const Udjat::XML::Node &node)
		: Udjat::NamedObject{node}, dialog_title{XML::QuarkFactory(node,"title")} {
	}

	Group::~Group() {
	}

	void Group::push_back(const Udjat::XML::Node &, std::shared_ptr<Udjat::Abstract::Object>) {
	}

 }

