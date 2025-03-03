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
  * @brief Implements abstract progress dialog.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/progress.h>
 #include <udjat/tools/logger.h>

 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Dialog::Progress * Dialog::Progress::instance = nullptr;

	Dialog::Progress::Progress() {
		instance = this;
	}

	Dialog::Progress::~Progress() {
		if(instance == this) {
			instance = nullptr;
		}
	}

	Dialog::Progress & Dialog::Progress::getInstance() {
		if(!instance) {
			throw logic_error("No active progress dialog");
		}
		return *instance;
	}

	Dialog::Progress & Dialog::Progress::title(const char *) {
		return *this;
	}

	std::shared_ptr<Dialog::Progress> Dialog::Progress::Factory() {
		return Controller::getInstance().ProgressFactory();
	}

	void Dialog::Progress::set(uint64_t current, uint64_t total) {

		if(total && current) {
			*this = ((double) current) / ((double) total);
		} else {
			*this = 0.0;
		}

	}

	int Dialog::Progress::run(const std::function<int(Progress &progress)> &task) noexcept {

		try {

			return task(*this);

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error("dialog");

		} catch(...) {

			Logger::String{"Unexpected error running background task"}.error("dialog");

		}

		return -1;

	}

	Dialog::Progress & Dialog::Progress::operator = (const double) {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::message(const char *) {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::body(const char *) {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::icon_name(const char *) {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::url(const char *) {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::file_sizes(const uint64_t, const uint64_t ) {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::item(const size_t, const size_t) {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::show() {
		return *this;
	}

	Dialog::Progress & Dialog::Progress::hide() {
		return *this;
	}

 }
