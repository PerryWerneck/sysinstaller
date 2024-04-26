/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <vector>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	void Dialog::setup(const XML::Node &node) {

#if UDJAT_CHECK_VERSION(1,2,0)
		message = XML::QuarkFactory(node,"message");
		secondary = XML::QuarkFactory(node,"secondary");

		for(auto parent = node;!(title && *title) && parent;parent = parent.parent()) {
			title = XML::QuarkFactory(parent,"title");
		}

		for(auto parent = node;!(icon && *icon) && parent;parent = parent.parent()) {
			icon = XML::QuarkFactory(parent,"icon-name");
		}
#else
		message = XML::QuarkFactory(node,"message").c_str();
		secondary = XML::QuarkFactory(node,"secondary").c_str();

		for(auto parent = node;!(title && *title) && parent;parent = parent.parent()) {
			title = XML::QuarkFactory(parent,"title").c_str();
		}

		for(auto parent = node;!(icon && *icon) && parent;parent = parent.parent()) {
			icon = XML::QuarkFactory(parent,"icon-name").c_str();
		}
#endif
		debug("-----------------------------------------------------------------------------------------");
		debug("title=",title);
		debug("message=",title);
		debug("icon=",icon);

		if(!(secondary && *secondary)) {
			Udjat::String text{node.child_value()};
			text.strip();
			if(!text.empty()) {
				text.expand(node,node.attribute("settings-from").as_string("dialog-defaults"));
				auto lines = text.split("\n");
				text.clear();
				for(String &line : lines) {
					line.strip();
					if(!text.empty()) {
						text += "\n";
					}
					text += line;
				}
				secondary = text.as_quark();
			}
		}
	}

	bool Dialog::setup(const char *name, const XML::Node &base) {

		for(XML::Node node = base; node; node = node.parent()) {
			for(XML::Node child = node.child("dialog"); child; child = child.next_sibling("dialog")) {
				if(strcasecmp(child.attribute("name").as_string("unnamed"),name) == 0) {
					debug("Loading dialog '",name,"'");
					setup(child);
					return true;
				}
			}
		}

		// Not found, set only the needed properties.
		Logger::String{"Cant find settings for dialog '",name,"', using defaults"}.trace("ui");

#if UDJAT_CHECK_VERSION(1,2,0)
		icon = XML::QuarkFactory(base,"icon-name");
		title = XML::QuarkFactory(base,"title");
#else
		icon = XML::QuarkFactory(base,"icon-name").c_str();
		title = XML::QuarkFactory(base,"title").c_str();
#endif // UDJAT_CHECK_VERSION

		return false;
	}

	bool Dialog::confirm(const char *yes, const char *no) const {
		if(message && *message) {
			debug(message);
			return Controller::instance().confirm(*this,yes,no);
		}
		Logger::String{"Confirmation dialog without message, assuming 'yes'"}.warning("dialogs");
		return true;
	}

	int Dialog::run(const std::vector<Dialog::Button> &buttons) const {
		return Controller::instance().run(*this,buttons);
	}

	int Dialog::run(const std::function<int(Progress &progress)> &task) const {
		return Controller::instance().run(*this,task);
	}

	int Dialog::run(const std::function<int(Popup &popup)> &task, const std::vector<Button> &buttons) const {
		return Controller::instance().run(*this,task,buttons);
	}

 }
