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
 #include <memory>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	std::shared_ptr<Dialog> Dialog::Factory(const char *name, const Udjat::XML::Node &node) {

		for(auto parent = node;parent;parent = parent.parent()) {
			for(auto child = parent.child("dialog");child;child = child.next_sibling("dialog")) {
				if(strcasecmp(XML::StringFactory(child,"name"),name) || !is_allowed(child)) {
					continue;
				}

				return make_shared<Dialog>(child);
			}
		}
		Logger::String{"Cant find dialog '",name,"', building an empty one"}.warning(node.attribute("name").as_string());
		return std::shared_ptr<Dialog>();
	}

	Dialog::Dialog(const Udjat::XML::Node &node, const Dialog::Option o) : options{o}, title{XML::QuarkFactory(node,"dialog-title")}  {

		if(!title || !*title) {
			title = XML::QuarkFactory(node,"title");
		}

		//
		// Load options.
		//
		static const struct {
			Dialog::Option value;
			const char * attrname;
		} opts[] {
			{ AllowQuitApplication,	"allow-quit" 	},
			{ AllowReboot,			"allow-reboot"	},
			{ NonInteractiveQuit,	"force-quit" 	},
			{ NonInteractiveReboot,	"force-reboot" 	},
		};

		for(const auto &option : opts) {
			if(XML::AttributeFactory(node,option.attrname).as_bool( (options & option.value) != 0)) {
				options = (Option) (options | option.value);
			} else {
				options = (Option) (option.value & ~option.value);
			}
		}
		
	}

	void Dialog::set(const Option value) {
		options = (Option) (options | value);
	}

 }

 /*
 #error deprecated
 
 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/ui/dialog.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <udjat/tools/configuration.h>

 #ifdef HAVE_UDJAT_DBUS
	#include <udjat/tools/dbus/connection.h>
	#include <udjat/tools/dbus/message.h>
 #endif // HAVE_UDJAT_DBUS

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	static Dialog::Controller *instance = nullptr;

	Dialog::Option Dialog::defoptions = Dialog::None;

	Dialog::Controller::Controller() {
		if(instance) {
			throw logic_error("Dialog controller is already active");
		}
		instance = this;
	}

	Dialog::Controller::~Controller() {
		instance = nullptr;
	}

	std::string Dialog::message(const char *def) const {
		if(args.message && *args.message) {
			return args.message;
		}
		return Config::Value<string>{ (string{"dialog-"} + args.name).c_str(), "message", def};
	}

	std::string Dialog::details(const char *def) const {
		if(args.details && *args.details) {
			return args.details;
		}
		return Config::Value<string>{ (string{"dialog-"} + args.name).c_str(), "details", def};
	}

	static void get_attribute_if_not_exists(const XML::Node &node, const char *name, const char **value) {

		if(*value && **value) {
			return;
		}

		*value = XML::QuarkFactory(node,name);

		debug(node.attribute("name").as_string(),"(",name,")='",*value,"'");

	}

	Dialog::Dialog(const char *name, Dialog::Option o, const XML::Node &node) : options{o} {

		static const struct {
			Dialog::Option value;
			const char * attrname;
		} options[] {
			{ AllowQuitApplication,	"allow-quit" 	},
			{ AllowReboot,			"allow-reboot"	},
			{ NonInteractiveQuit,	"force-quit" 	},
			{ NonInteractiveReboot,	"force-reboot" 	},
		};

		args.name = Quark{name}.c_str();

		this->options = (Dialog::Option) (this->options | defoptions);

		for(auto parent = node;parent;parent = parent.parent()) {

			for(auto child = parent.child("dialog");child;child = child.next_sibling("dialog")) {

				if(strcasecmp(XML::StringFactory(child,"name"),name) || !is_allowed(child)) {
					continue;
				}

				for(const auto &option : options) {
					bool current = (this->options & option.value) != 0;
					auto attr = XML::AttributeFactory(child,option.attrname);
					if(attr) {
						debug("Found ",option.attrname);
						if(attr.as_bool(current)) {
							this->options = (Dialog::Option) (this->options | option.value);
						} else {
							this->options = (Dialog::Option) (this->options & ~option.value);
						}
					}
				}

				// Load dialog properties.
				get_attribute_if_not_exists(child,"icon",&args.icon_name);
				get_attribute_if_not_exists(child,"message",&args.message);

				if(!(args.details && *args.details)) {
					Udjat::String text{child.child_value()};
					text.expand(child);
					text.strip();
					args.details = text.as_quark();
					debug(child.attribute("name").as_string(),"(details)='",args.details,"'");
				}

			}

		}

	}

	Dialog::~Dialog() {
	}

	Dialog::Controller & Dialog::Controller::getInstance() {
		if(!instance) {

			// TODO: Build dummy dialog controller.
			throw logic_error("The dialog controller was not initialized");

		}

		return *instance;
	}

	void Dialog::quit() const noexcept {
		Logger::String{"Unable to quit application: ",strerror(ENOTSUP)}.error("dialog");
	}

	void Dialog::reboot() const noexcept {
#if defined(HAVE_UDJAT_DBUS)
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
#else
		Logger::String{"Unable to reboot system: ",strerror(ENOTSUP)}.error("dialog");
#endif // HAVE_UDJAT_DBUS

	}

	bool Dialog::confirm() const noexcept {
		if(*this) {
			return select(1, _("_No"),_("_Yes"),nullptr) == 0;
		}
		return true; // Dialog is invalid, always return 'true'
	}

	int Dialog::select(int cancel, const char *button, ...) const {
		int rc = -1;
		if(*this) {
			va_list args;
			va_start(args, button);
			rc = Controller::getInstance().select(*this,cancel,button,args);
			va_end(args);
		}
		return rc;
	}

 }
 */ 
