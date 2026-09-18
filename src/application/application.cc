/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/properties.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>
 #include <vector>
 #include <udjat/tools/string.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Application * Application::instance = nullptr;
	bool Application::non_interactive_mode = false;
	std::vector<String> Application::selected_path;

	Application::Application() : Properties::ObjectBuilder("group") {
		if(instance) {
			throw std::logic_error("Application instance already exists");
		}
		instance = this;
	}

	Application::~Application() {
		instance = nullptr;
	}

	int Application::run() {

		//
		// Load options
		//
#ifdef DEBUG
		Properties::parse(MimeType::xml,"./xml.d");
#else
		Properties::parse(MimeType::xml);
#endif

		//
		// Run user interaction.
		//	
		if(non_interactive_mode) {
			return run_non_interactive();
		}

		return run_interactive();

	}

	Application & Application::get_instance() {
		if(!instance) {
			throw std::logic_error("Application instance does not exist");
		}
		return *instance;
	}

	void Application::set_selected_path(const char *path) {
		if(!(path && *path)) {
			throw std::invalid_argument("Missing path");
		}
		selected_path.clear();
		String{path}.split(selected_path,"/");
	}

	bool Application::push_back(const Udjat::Properties &props, std::shared_ptr<Item> item) {

		for(const auto &itn : itens) {
			if(!strcasecmp(itn->c_str(),item->c_str())) {
				Logger::String{"Item '", item->c_str(), "' already exists"}.warning("groups");
				break;
			}
		}
		itens.push_back(item);

		if(selected_path.size() >=1) {
			if(strcasecmp(selected_path[0].c_str(),item->c_str())) {
				return false;
			}
			selected_item = item;
			return true;
		}

		if(props.get("default",false)) {
			selected_item = item;
			return true;
		}

		return false;
	}

	Application::Item::Item(const Udjat::Properties &props) : String{props["name"].c_str()} {
	
	}


 }
 