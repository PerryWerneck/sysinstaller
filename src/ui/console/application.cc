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
  * @brief Implements gtk4 toplevel.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/console.h>
 #include <vector>

 #ifdef LOG_DOMAIN
	#undef LOG_DOMAIN
 #endif
 #define LOG_DOMAIN "tui"
 #include <udjat/tools/logger.h>

 #include <udjat/tools/configuration.h>
 #include <udjat/tools/xml.h>
 #include <udjat/ui/console.h>
 #include <udjat/ui/status.h>
 #include <udjat/ui/progress.h>
 #include <udjat/tools/intl.h>

 #include <reinstall/action.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H	
 
 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	class Reinstall::Console::Group : public Reinstall::Group {
	public:

		static std::shared_ptr<Reinstall::Action> preset;

		struct Item {
			std::shared_ptr<Reinstall::Action> action; ///< @brief The action associated with this item.
			const char *label;

			Item(const Udjat::XML::Node &node, std::shared_ptr<Reinstall::Action> a) 
				: action{a},label{XML::QuarkFactory(node,"title",XML::AttributeFactory(node,"name").as_string("Unnamed action"))} {	
			}

		};
		vector<Item> itens; ///< @brief The list of actions in this group.	

		const char *label; ///< @brief The label for this group.

		Group(const Udjat::XML::Node &node) : label{XML::QuarkFactory(node,"title",XML::AttributeFactory(node,"name").as_string("Unnamed group"))} {
			debug("Creating group '",label,"'");
		}

		~Group() override = default;

		void push_back(const Udjat::XML::Node &node, std::shared_ptr<Reinstall::Action> action) override {

#if __cplusplus >= 201703L
			auto &itn = itens.emplace_back(node,action);
#else
			itens.emplace_back(node,action);
			auto &itn = itens.back();
#endif // C++17

			if(itn.action->is_default(node)) {
				preset = itn.action;
			}
		}

	};

	std::shared_ptr<Reinstall::Action> Reinstall::Console::Group::preset;

	Console::Console() {
		
		load_options();

	}

	Console::~Console() noexcept {

	}

	int Console::run(int argc, char *argv[]) {

		std::shared_ptr<Reinstall::Action> selected_action = Reinstall::Console::Group::preset;

		if(!(Action::has_preset() || Reinstall::Dialog::has_preset(Reinstall::Dialog::NonInteractive))) {

			// No present - Present main menu.
			UI::Console console;

			console << _("Available options") << endl << endl;

			// Select group.
			std::shared_ptr<Group> selected_group;
			{
				char item[] = "A";
				for(const auto &group : groups) {
					console << "\t";
					console.bold(true);
					console << item;
					console.bold(false);
					console << " - " << group->label << endl;
					item[0]++;
				}

				console << endl << _("Select group (Enter to quit): ");
				console.cursor(true).flush();

			}

			String choice;

			cin.sync(); 
			getline(cin, choice);
			debug("User choice: ", choice.c_str());

			{
				char item = 'A';
				char entry = toupper(choice[0]);
				for(const auto &group : groups) {
					console.erase_line().up();
					if(item++ == entry) {
						selected_group = group;
					}
				}

				for(size_t line = 0; line < 4; line++) {
					console.erase_line().up();
				}

			}

			if(choice.empty()) {
				Logger::String{"User selected quit option"}.info();
				return ECANCELED; // Quit.
			}

			if(!selected_group) {
				Logger::String{"User selected an invalid group"}.info();
				return EINVAL; // Invalid choice.
			}

			console.bold(true);
			console << selected_group->label << ":" << endl << endl;
			console.bold(false);

			// Select item.
			{
				char option[] = "A";
				for(const auto &item : selected_group->itens) {
					console << "\t";
					console.bold(true);
					console << option;
					console.bold(false);
					console << " - " << item.label << endl;
					option[0]++;
				}

				console << endl << _("Select action (Enter to quit): ");
				console.cursor(true).flush();
				
			}

			cin.sync(); 
			getline(cin, choice);

			{
				char item = 'A';
				char entry = toupper(choice[0]);
				for(const auto &itn : selected_group->itens) {
					console.erase_line().up();
					if(item++ == entry) {
						selected_action = itn.action;
					}
				}

				for(size_t line = 0; line < 3; line++) {
					console.erase_line().up();
				}

			}

			if(choice.empty()) {
				Logger::String{"User selected quit option"}.info();
				return ECANCELED; // Quit.
			}

			if(!selected_action) {
				Logger::String{"User selected an invalid action"}.info();
				return EINVAL; // Invalid choice.
			}

			console.bold(true);
			console << selected_action->title() << endl << endl;
			console.bold(false);

			if(!selected_action->confirmation->ask(true)) {
				Logger::String{"User cancelled action"}.info();
				return ECANCELED; // User cancelled.
			}

			Logger::String{"User selected '",selected_action->title(),"'"}.info();

		}

		if(!selected_action) {
			Logger::String{"No action selected"}.error();
			return -EINVAL; // No action selected.
		}

		try {

			selected_action->activate();
			Logger::String{"Action '",selected_action->title(),"' executed successfully"}.info();

			selected_action->success->present();

		} catch(const std::exception &e) {
			Logger::String{"Action '",selected_action->title(),"' has failed"}.error();
			debug(e.what());
			selected_action->failed->present(e);
			return -1;
		}

		debug("Console action completed successfully");
		return 0;
	}

	std::shared_ptr<Reinstall::Group> Console::group_factory(const Udjat::XML::Node &node) {
		auto group = make_shared<Group>(node);
		groups.push_back(group);
		return group;
	}

	void Console::failed(const std::exception &e) noexcept {

	}

	std::shared_ptr<Reinstall::Dialog> Console::DialogFactory(const char *name, const Udjat::XML::Node &node, const char *message, const Dialog::Option option) {

		class Dialog : public Reinstall::Dialog {
		public:

			Dialog(const Udjat::XML::Node &node, const char *msg, const Option option) : Reinstall::Dialog{node,msg,option} {
			}

			virtual ~Dialog() {
			}

			bool ask(bool default_response) const noexcept override {
				return default_response;
			}

			bool present(const char *msg) const noexcept override {

				if(Reinstall::Dialog::present(msg)) {
					Logger::String{((msg && *msg) ? msg : message)}.info();
					return true;
				}

				Udjat::UI::Console console;

				console.bold(true);
				console << message << endl;
				console.bold(false);

				if(msg && *msg) {
					console << msg << endl;
				}

				console << endl;



				console << ":";

				String choice;
				console.cursor(true).flush();
				cin.sync(); 
				getline(cin, choice);

				return true;
			}

		};

		return make_shared<Dialog>(node,message,option);

	}	

 }
 