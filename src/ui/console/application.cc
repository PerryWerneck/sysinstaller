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
 #include <udjat/tools/intl.h>

 #include <reinstall/action.h>
 
 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	class Reinstall::Console::Group : public Reinstall::Group {
	public:

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
			setup(node);
		}

		~Group() override = default;

		void push_back(const Udjat::XML::Node &node, std::shared_ptr<Reinstall::Action> action) override {
			itens.emplace_back(node,action);
		}

	};

	Console::Console() {
		
		load_options();

	}

	Console::~Console() noexcept {

	}

	int Console::run(int argc, char *argv[]) {

		std::shared_ptr<Reinstall::Action> selected_action;
		
		{
			// Present main menu.
			std::shared_ptr<Group> selected_group;

			UI::Console console;

			console << _("Available options") << endl << endl;

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

				for(size_t line = 0; line < 4; line++) {
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

			if(!selected_action->confirmation->ask(true)) {
				Logger::String{"User cancelled action"}.info();
				return ECANCELED; // User cancelled.
			}
		}

		Logger::String{"User selected '",selected_action->title(),"'"}.info();

		return -1;
	}

	std::shared_ptr<Reinstall::Group> Console::group_factory(const Udjat::XML::Node &node) {
		auto group = make_shared<Group>(node);
		groups.push_back(group);
		return group;
	}

	void Console::failed(const std::exception &e) noexcept {

	}

	std::shared_ptr<Reinstall::Dialog> Console::DialogFactory(const char *name, const Udjat::XML::Node &node, const char *message, const Dialog::Option option) {
		return make_shared<Reinstall::Dialog>(node, message, option);
	}	

 }
 