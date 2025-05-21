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
 #include <reinstall/application.h>
 #include <udjat/tools/factory.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/url/handler/http.h>
 #include <udjat/tools/configuration.h>
 #include <string>
 #include <reinstall/action.h>

 #include <reinstall/modules/grub2.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Application *Application::instance = nullptr;
	static const Udjat::ModuleInfo moduleinfo{"Top menu option"};
		
	Application::Application() : Factory{"group",moduleinfo} {

		// Load default http handler.
		static HTTP::Handler::Factory http_handler{"default"};

		if(instance) {
			throw std::runtime_error{"Application already created"};
		}

		instance = this;
		Logger::String{"Creating application"}.info();

		// Load modules.

		if(Config::Value<bool>{"modules","grub2",true}) {
			Reinstall::Grub2::Module::Factory("grub");
		}

	}

	Application::~Application() {
		instance = nullptr;
	}

	Application & Application::getInstance() {
		if(!instance) {
			throw std::runtime_error{"Application not created"};
		}
		return *instance;
	}

	void Application::load_options() {

		Logger::String{"Loading options"}.info();

	#ifdef DEBUG
		XML::parse("./xml.d");
	#else
		XML::parse();
	#endif

	}

	void Application::push_back(const Udjat::XML::Node &node, std::shared_ptr<Reinstall::Action> child) {

		debug("Adding action '",child->name(),"'");

		string parent{node.parent().attribute("name").as_string("default")};

		auto result = groups.find(parent);
		if(result == groups.end()) {
			throw runtime_error{Logger::String{"Group '",parent.c_str(),"' not found"}};
		}

		result->second->push_back(node,child);

	}

	bool Application::parse(const Udjat::XML::Node &node) {

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

		return true;
	}
	
 }
 