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
  * @brief Implements abstract dialog.
  */

 #include <config.h>
 #include <reinstall/dialog.h>
 #include <reinstall/application.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <memory>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Dialog::Option Dialog::presets = Dialog::None;

	std::shared_ptr<Dialog> Dialog::Factory(const char *name, const Udjat::XML::Node &node, const char *message, const Option option) {

		debug("Searching for dialog '",name,"' in ",node.attribute("name").as_string());

		for(auto parent = node;parent;parent = parent.parent()) {
			for(auto child = parent.child("dialog");child;child = child.next_sibling("dialog")) {
				if(strcasecmp(XML::StringFactory(child,"name"),name) || !is_allowed(child)) {
					continue;
				}

				debug("==============================> Found dialog '",name,"' in ",child.attribute("name").as_string());
				return Application::getInstance().DialogFactory(name,child,message,option);
			}
		}
		Logger::String{"Cant find dialog '",name,"', building from default config"}.warning("dialog");

		XML::Node defnode;
		defnode.attribute("name").set_value(name);

		Config::for_each(String{"dialog-",name}.c_str(),[&defnode](const char *key, const char *value) -> bool{
			Logger::String{"Setting '",key,"' to '",value,"'"}.trace("dialog");
			defnode.attribute(key).set_value(value);
			return false;
		});

		return Application::getInstance().DialogFactory(name,defnode,message,option);

	}

	Dialog::Option Dialog::OptionFactory(const char *name) {
		static const struct {
			const char *name;
			Option option;
		} button_names[] = {
			{ "reboot", 	Reboot		},
			{ "continue", 	Continue	},
			{ "quit", 		Quit		},
			{ "cancel", 	Cancel		},
		};
		for(const auto &button : button_names) {
			if(strcasecmp(name,button.name) == 0) {
				return button.option;
			}
		}
		return Option::None;
	}

	Dialog::Buttons::Buttons(const XML::Node &node) {
		for(const auto &button : String{XML::StringFactory(node,"button-order","continue,quit,cancel,reboot")}.split(",")) {
			auto opt = Dialog::OptionFactory(button.c_str());
			if(opt != Option::None) {
				order.push_back(opt);
			}
		}
		destructive = Dialog::OptionFactory(XML::StringFactory(node,"destructive-button","reboot"));
		suggested = Dialog::OptionFactory(XML::StringFactory(node,"suggested-button","none"));
	}

	Dialog::Dialog(const Udjat::XML::Node &node, const char *msg, const Option o) 
		: options{(Option) (o|presets)}, 
			buttons{node},
			title{XML::QuarkFactory(node,"dialog-title")},
			message{XML::QuarkFactory(node,"message",msg)},
			destructive{XML::AttributeFactory(node,"destructive").as_bool(false)}  {

		details = String{node.child_value()}.strip().as_quark();

		debug("Dialog title set to '",title,"'");
		debug("Dialog message set to '",message,"'");
		debug("Dialog details set to '",details,"'");
		debug("Dialog options set to ",options);

		//
		// Load options.
		//
		static const struct {
			Dialog::Option value;
			const char * attrname;
		} opts[] {
			{ Quit,					"allow-quit" 				},
			{ Reboot,				"allow-reboot"				},	
			{ NonInteractiveQuit,	"force-quit" 				},
			{ NonInteractiveReboot,	"force-reboot" 				},
			{ NonInteractiveQuit,	"non-interactive-quit" 		},
			{ NonInteractiveReboot,	"non-interactive-reboot" 	},
		};

		for(const auto &option : opts) {
			if(XML::AttributeFactory(node,option.attrname).as_bool( (options & option.value) != 0)) {
				options = (Option) (options | option.value);
				debug("Option ",option.attrname," is set (",options,")");
			} else {
				options = (Option) (options & ~option.value);
				debug("Option ",option.attrname," is not set (",options,")");
			}
		}
		
		debug("Dialog options updated to ",options);

	}

	void Dialog::set(const Option value) {
		options = (Option) (options | value);
	}

	void Dialog::preset(const Option value) noexcept {
		presets = (Option) (presets | value);
	}

	bool Dialog::ask(bool default_response) const noexcept {
		return default_response;
	}
	
	bool Dialog::present(const char *) const noexcept {

		/*
		if(has(NonInteractiveQuit)) {
			Logger::String{"Non-interactive dialog, quitting"}.info();
			quit();
			return true; // Non-interactive dialogs always return true.
		} else if(has(NonInteractiveReboot)) {
			Logger::String{"Non-interactive dialog, rebooting"}.info();
			reboot();
			return true; // Non-interactive dialogs always return true.
		}
		*/

		debug("Dialog::present() called in ",(has(NonInteractive) ? "non-interactive" : "interactive")," mode");

		return has(NonInteractive);
	}

	const char * Dialog::text(const char *def) const noexcept {
		if(message && *message) {
			return message;
		}
		return def;
	}

	const char * Dialog::body(const char *def) const noexcept {
		if(details && *details) {
			return details;
		}
		return def;
	}

	void Dialog::quit() const {
		debug("Dialog::quit() called without implementation");
	}

	void Dialog::cancel() const {

	}

	void Dialog::reboot() const {
#ifdef DEBUG 
		debug('---------------- Rebooting system ----------------');
		quit();
#else
		try {
			// Ask logind for reboot
			DBus::SystemBus{}.call(
				DBus::Message{
						"org.freedesktop.login1",
						"/org/freedesktop/login1",
						"org.freedesktop.login1.Manager",
						"Reboot",
						true
				},
				[this](Udjat::DBus::Message &response){
					if(response) {
						Logger::String{"Reboot request sent to logind"}.info("dialog");
						quit();
					} else {
						Logger::String{"Error '",response.error_message(),"' sending reboot request to logind"}.error("dialog");
					}
				}
			);

		} catch(const std::exception &e) {
			Logger::String{"Error '",e.what(),"' sending reboot request to logind"}.warning("dialog");
		}
#endif // DEBUG

	}

 }
