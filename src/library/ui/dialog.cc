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
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	static Dialog::Controller *instance = nullptr;

	Dialog::Controller::Controller() {
		if(instance) {
			throw logic_error("Dialog controller is already active");
		}
		instance = this;
	}

	Dialog::Controller::~Controller() {
		instance = nullptr;
	}

	Dialog::Dialog() {
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

	void Dialog::title(const char *) {
	}

	/*
	int Dialog::Controller::run(Dialog *, const std::function<int()> &task) noexcept {

		try {

			return task();

		} catch(const std::exception &e) {

			Logger::String{"Background task failed: ",e.what()}.error("dialog");

		} catch(...) {

			Logger::String{"Unexpected error running background task"}.error("dialog");

		}

		return -1;

	}
	*/

 }
