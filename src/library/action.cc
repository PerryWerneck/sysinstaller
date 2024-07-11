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
 #include <udjat/ui/progress.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/configuration.h>

 #include <stdexcept>

 #include <unistd.h> // sleep

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Action::Action(const Udjat::Abstract::Object &object, const Udjat::XML::Node &node)
		: NamedObject{node}, parent{dynamic_cast<const Group *>(&object)},
		 args{node}, confirmation{"confirmation",Dialog::Option::None,node},
		 success{"success",Dialog::Option::AllowQuitContinue,node},
		 failed{"failed",Dialog::Option::AllowQuitContinue,node} {

		debug("Building action '",name(),"' on group '",object.name(),"'");
		if(!parent) {
			throw logic_error("Action parent should be a group controller");
		}

		if(!confirmation) {
			Logger::String{"Confirmation dialog is not defined, disabling it"}.trace(name());
		}

		if(!success) {
			Logger::String{"Success dialog is not defined, disabling it"}.trace(name());
		}

		if(!failed) {
			Logger::String{"Failed dialog is not defined, disabling it"}.trace(name());
		}

	}

	Action::~Action() {
		debug("Destroying action '",name(),"'");
	}

	int Action::activate(Udjat::Dialog::Progress &) {
		throw logic_error(_("The selected action cant be activated"));
	}

	bool Action::initialize() {
		return true;
	}

	void Action::activate() {

		if(confirmation && confirmation.select(0,_("Cancel"),_("Continue"),nullptr) != 1) {
			info() << "Operation cancelled by user" << endl;
			return;
		}

		//
		// Run action
		//
		auto dialog = Udjat::Dialog::Progress::Factory();

		info() << "Starting activity" << endl;

		dialog->title(group().title());
		dialog->message(args.title);
		dialog->body(_("Initializing"));
		dialog->icon_name(args.icon_name);

		struct {
			string message;
			string details;
		} popup;

		auto rc = dialog->run([&](Udjat::Dialog::Progress &progress){

			try {

				activate(progress);
				return 0;

			} catch(const Udjat::Exception &e) {

				popup.message = e.title();
				popup.details = e.body();
				Logger::String{e.body()}.error(name());

			} catch(const std::logic_error &e) {

				popup.message = _("Configuration or logic error");
				popup.details = e.what();
				Logger::String{e.what()}.error(name());

			} catch(const std::system_error &e) {

				Logger::String details{e.what()," (rc=",e.code().value(),")"};
				details.error(name());

				popup.message = _("System error");
				popup.details = details;

			} catch(const std::exception &e) {

				popup.message = _("Operation failed");
				popup.details = e.what();
				Logger::String{e.what()}.error(name());

			}

			return -1;
		});

		if(rc) {
			failed.present(popup.message.c_str(),popup.details.c_str());
		} else if(success) {
			Logger::String{success.details()}.info(name());
			success.present(popup.message.c_str(),popup.details.c_str());
		} else {
			Logger::String{"Completed, no success dialog"}.info(name());
		}

	}

	bool Action::getProperty(const char *key, std::string &value) const {

		/*
		if(!strcasecmp(key,"kernel-name")) {
			value = Config::Value<string>("app-defaults","kernel-name","kernel.reinstall");
			return true;
		}

		if(!strcasecmp(key,"init-name")) {
			value = Config::Value<string>("app-defaults","kernel-name","initrd.reinstall");
			return true;
		}
		*/

		if(Udjat::NamedObject::getProperty(key,value)) {
			return true;
		}

		Logger::String{"Unable to expand property '",key,"'"}.warning(name());

		return false;
	}


 }

