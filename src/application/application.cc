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
  * @brief The reinstall application.
  */

 #include <config.h>
 #include <private/application.h>
 #include <udjat/tools/factory.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <string>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	static const Udjat::ModuleInfo moduleinfo{"Top menu option"};
		
	Application::Application() : Factory{"group",moduleinfo} {

	}

	Application::~Application() {

	}

	void Application::load_options() {

		Logger::String{"Loading options"}.info();

	}

	void Application::select(std::shared_ptr<Reinstall::Group> group) noexcept {
		selected = group;
	}

	bool Application::NodeFactory(const Udjat::XML::Node &node) {

		string name{node.attribute("name").as_string("default")};
		std::shared_ptr<Group> group;

		auto result = groups.find(name);
		if(result == groups.end()) {
			Logger::String{"Building group '",name.c_str(),"'"}.trace();
			group = group_factory(node);
			groups.insert({name,group});
		} else {
			Logger::String{"Updating group '",name.c_str(),"'"}.trace();
			group = result->second;
		}

		group->setup(node);
		
		if(node.attribute("default").as_bool(false)) {
			select(group);
		}

		return true;
	}
	
 }
 