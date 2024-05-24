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
  * @brief Implement mainloop bindings to gtk4 mainloop.
  */

 #include <config.h>

 #include <gtkmm.h>
 #include <map.h>

 #include <udjat/defs.h>
 #include <udjat/tools/timer.h>
 #include <udjat/ui/gtk4/mainloop.h>

 using namespace std;

 class UDJAT_PRIVATE Timers {
 private:

 	// References:
 	//
 	// https://github.com/GNOME/gtkmm-documentation/blob/master/examples/book/timeout/timerexample.cc
 	//

	struct Entry {
		sigc::connection connection;
		MainLoop::Timer *timer;

		Entry(MainLoop::Timer *timer) {

			connection = Glib::signal_timeout().connect([this](){

				unsigned long new_value = timer->activate();

			},1);

		}
	};

 	Timers() {
 	};

 public:
	static Timers & getInstance() {
		static Timers instance;
		return instance;
	}

	void push_back(MainLoop::Timer *timer) {

	}

	void remove(MainLoop::Timer *timer) {
	}

 };

 namespace Udjat {

	void Gtk::MainLoop::push_back(MainLoop::Timer *timer) {
		Timers::getInstance().push_back(timer);
	}

	void Gtk::MainLoop::remove(MainLoop::Timer *timer) {
		Timers::getInstance().remove(timer);
	}

 }
