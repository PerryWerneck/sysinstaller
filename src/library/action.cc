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

 #include <stdexcept>

 #include <unistd.h> // sleep

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Action::Action(const Udjat::Abstract::Object &object, const Udjat::XML::Node &node)
		: NamedObject{node}, parent{dynamic_cast<const Group *>(&object)},
		 args{node}, confirmation{node,"confirmation"}, success{node,"success"}, failed{node,"failed"} {

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
		throw runtime_error("Invalid action");
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

		debug("group-title=",group().title());

		info() << "Starting activity" << endl;

		dialog->title(group().title());
		dialog->message(args.title);
		dialog->body(_("Initializing"));
		dialog->icon_name(args.icon_name);

		dialog->run([&](Udjat::Dialog::Progress &progress){

			try {

				activate(progress);
				return 0;

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error(name());

				// TODO: Show error popup.
			}

			return -1;
		});

	}

 }

