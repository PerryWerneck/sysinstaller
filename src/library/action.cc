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
 #include <reinstall/dialog.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/application.h>
 #include <stdexcept>
 #include <reinstall/dialog.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/xml.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	const char * Action::presets[2] = {nullptr,nullptr};


	Model::Model(const Udjat::XML::Node &node) {

		String name{node,"model"};
		if(name.empty()) {
			return;
		}

		Logger::String{"Loading model '",name,"' for node ",node.path()}.info();

#ifdef DEBUG
		String path{getenv("PWD")};
		path += "/models/";
#else
		String path{Application::DataDir{"models"}.c_str()};
#endif // DEBUG

		path += name;

		debug("Model path: ",path.c_str());

		XML::Document{path.c_str()}.copy_to(*(const_cast<XML::Node *>(&node)));

	}

	Action::Action(const Udjat::XML::Node &node) 
		: Model{node}, NamedObject{node}, 
			dialog_title{XML::QuarkFactory(node,"dialog-title")},
		 	icon_name{XML::QuarkFactory(node,"icon-name")} {
		
		if(!(dialog_title && *dialog_title)) {
			dialog_title = XML::QuarkFactory(node,"title");
		}

		if(!(icon_name && *icon_name)) {
			icon_name = XML::QuarkFactory(node,"icon");
		}	

		confirmation = Dialog::Factory("confirmation",node,_("Do you confirm?"));
		success = Dialog::Factory("success",node,_("Operation complete"),Dialog::QuitContinue);
		failed = Dialog::Factory("failed",node,_("Operation failed"),Dialog::QuitContinue);

	}

	Action::~Action() {

	}

	/// @brief Activate the action, called on selected action when the 'apply' button is pressed.
	void Action::activate() {
		Logger::String{"This action cant be activated"}.error(name());	
		throw logic_error(_("The selected action cant be activated"));
	}

	/// @brief Test if the action is valid and can be activated.
	bool Action::initialize() {
		return true;
	}

	void Action::preset(const char *value) {

		auto args = Udjat::String{value}.split("/",2);
		if(args.size() != 2) {
			throw std::runtime_error{Logger::Message(_("Unable to select '{}': Invalid path"),value)};
		}

		presets[0] = args[0].as_quark();
		presets[1] = args[1].as_quark();
		
		Logger::String{"Setting preset to '",presets[0],"/",presets[1],"'"}.info();
	}

	bool Action::is_default(const Udjat::XML::Node &node) noexcept {

		bool has_preset = Action::has_preset();
		if(has_preset) {
			if(has_preset && strcasecmp(presets[0],node.parent().attribute("name").as_string("default"))) {
				return false;
			}

			if(has_preset && strcasecmp(presets[1],node.attribute("name").as_string("default"))) {
				return false;
			}			
		}

		return XML::AttributeFactory(node,"default").as_bool(has_preset) || 
		       XML::AttributeFactory(node,"selected").as_bool(has_preset) || 
		       XML::AttributeFactory(node,"active").as_bool(has_preset);

	}

 }
 
