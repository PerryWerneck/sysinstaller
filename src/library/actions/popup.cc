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
  * @brief Implements the popup dialog for reinstall actions.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/dialog.h>
 #include <libreinstall/action.h>
 #include <vector>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	void Popup::setup(const XML::Node &node) {

		Udjat::Dialog::setup(node);

#if UDJAT_CHECK_VERSION(1,2,0)
		allow_reboot = XML::AttributeFactory(node,"allow-reboot").as_bool();
		allow_quit = XML::AttributeFactory(node,"allow-quit").as_bool();
#else
		allow_reboot = XML::StringFactory(node,"allow-reboot").as_bool();
		allow_quit = XML::StringFactory(node,"allow-quit").as_bool();
#endif // UDJAT_CHECK_VERSION

	}

	void Popup::run(const char *error_message, bool allow_close) const {

		Popup dialog = *this;

		if(!(dialog.message && *dialog.message)) {
			dialog.message = _("Operation has failed");
		}

		dialog.secondary = error_message;
		dialog.run(allow_close);

	}

	void Popup::run(bool allow_close) const {

		vector<Button> buttons;

		if(allow_close) {
			buttons.emplace_back(0,_("_Close"));
		}

		if(allow_quit) {
			buttons.emplace_back(1,_("_Quit application"));
		}

		if(allow_reboot) {
			buttons.emplace_back(2,_("_Reboot"));
		}

		switch(Dialog::Controller::instance().run(*this,buttons)) {
		case 0:	// Close
			break;

		case 1: // Quit application.
			Dialog::Controller::instance().quit();
			break;

		case 2: // Reboot
			break;
		}

	}

 }

