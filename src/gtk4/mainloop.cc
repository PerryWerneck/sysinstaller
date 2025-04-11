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

/*
 #include <config.h>

 #include <gtkmm.h>

 #include <udjat/defs.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/handler.h>
 #include <reinstall/ui/mainloop.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <list>
 #include <stdexcept>

 using namespace std;
 using namespace Udjat;

 // References:
 //
 //		https://gnome.pages.gitlab.gnome.org/glibmm/group__MainLoop.html

 class UDJAT_PRIVATE Timers {
 private:

 	// References:
 	//
 	// https://github.com/GNOME/gtkmm-documentation/blob/master/examples/book/timeout/timerexample.cc
 	//

	struct Entry {

		sigc::connection connection;
		Udjat::MainLoop::Timer *timer;

		Entry(Udjat::MainLoop::Timer *t) : timer{t} {

			debug("Timer enabled");
			connection = Glib::signal_timeout().connect([this](){

				try {

					timer->activate();

				} catch(const std::exception &e) {

					cerr << "MainLoop\t" << e.what() << endl;

				} catch(...) {

					cerr << "MainLoop\tUnexpected error activating timer " << endl;
				}

				return true;

			},timer->interval());

		}

		bool operator==(const Entry &entry) const noexcept {
			return entry.timer == this->timer;
		}

		~Entry() {
			connection.disconnect();
			debug("Timer disabled");
		}

	};

	list<Entry> entries;

 	Timers() {
 	};

 public:
	static Timers & getInstance() {
		static Timers instance;
		return instance;
	}

	void push_back(Udjat::MainLoop::Timer *timer) {
		entries.emplace_back(timer);
	}

	void remove(Udjat::MainLoop::Timer *timer) {
		entries.remove(timer);
	}

	bool enabled(const Udjat::MainLoop::Timer *timer) const noexcept {
		for(const Entry &entry : entries) {
			if(entry.timer == timer) {
				return true;
			}
		}
		return false;
	}

 };


 static const struct EventCode {
	MainLoop::Handler::Event event;
	Glib::IOCondition condition;
 } eventcodes[] = {
	{ MainLoop::Handler::oninput,	Glib::IOCondition::IO_IN	},
	{ MainLoop::Handler::oninput,	Glib::IOCondition::IO_PRI	},
	{ MainLoop::Handler::onoutput,	Glib::IOCondition::IO_OUT	},
	{ MainLoop::Handler::onerror,	Glib::IOCondition::IO_ERR	},
	{ MainLoop::Handler::onhangup,	Glib::IOCondition::IO_HUP	},
 };

 static Glib::IOCondition IOConditionFactory(MainLoop::Handler::Event event) noexcept {
	Glib::IOCondition condition = (Glib::IOCondition) 0;
	for(const auto &eventcode : eventcodes) {
		if(event & eventcode.event) {
			condition |= eventcode.condition;
		}
	}
	return condition;
 }

 MainLoop::Handler::Event EventFactory(const Glib::IOCondition condition) noexcept {
	MainLoop::Handler::Event event = (MainLoop::Handler::Event) 0;
	for(const auto &eventcode : eventcodes) {
		if(condition == eventcode.condition) {
			event = (MainLoop::Handler::Event) (event|eventcode.event);
		}
	}
	return event;
 }

 class UDJAT_PRIVATE Handlers {
 private:


 	struct Entry : public Glib::IOSource {

		sigc::connection connection;
		Udjat::MainLoop::Handler *handler;

		Entry(Udjat::MainLoop::Handler *h) : Glib::IOSource{h->fd(),IOConditionFactory(h->events())}, handler{h} {

			connection = connect([this](Glib::IOCondition condition){
				handler->handle(EventFactory(condition));
				return true;
			});

		}

		~Entry() {
			connection.disconnect();
		}

		bool operator==(const Entry &entry) const noexcept {
			return entry.handler == this->handler;
		}

	};

	list<Entry> entries;

 	Handlers() {
 	}

 public:
	static Handlers & getInstance() {
		static Handlers instance;
		return instance;
	}

	void push_back(Udjat::MainLoop::Handler *handler) {
		entries.emplace_back(handler);
	}

	void remove(Udjat::MainLoop::Handler *handler) {
		entries.remove(handler);
	}

	bool enabled(const Udjat::MainLoop::Handler *handler) const noexcept {
		for(const Entry &entry : entries) {
			if(entry.handler == handler) {
				return true;
			}
		}
		return false;
	}

 };

 namespace Reinstall {

 	//
 	// MainLoop
 	//
	MainLoop::MainLoop() : Udjat::MainLoop{Udjat::MainLoop::GLib} {
		debug("Activating GLIB based mainloop");
	}

	MainLoop::~MainLoop() {
	}

	void MainLoop::wakeup() noexcept {
	}

	int MainLoop::run() {
		throw std::system_error(ENOTSUP,std::system_category(),"Use Gtk::Application::run");
	}

	void MainLoop::quit() {
		::Gtk::Application::get_default()->quit();
	}

	bool MainLoop::active() const noexcept {
		return true;
	}

	static gboolean on_idle_call(gpointer user_data) noexcept {
		Udjat::MainLoop::Message::on_posted((Udjat::MainLoop::Message *) user_data);
		return G_SOURCE_REMOVE;
	}
	
	void MainLoop::post(Udjat::MainLoop::Message *message) noexcept {
		g_idle_add((GSourceFunc) on_idle_call, (gpointer) message);
	}

	//
	// Timers
	//
	bool MainLoop::enabled(const Timer *timer) const noexcept {
		return Timers::getInstance().enabled(timer);
	}

	void MainLoop::push_back(MainLoop::Timer *timer) {
		Timers::getInstance().push_back(timer);
	}

	void MainLoop::remove(MainLoop::Timer *timer) {
		Timers::getInstance().remove(timer);
	}

	//
	// Handlers
	//
	void MainLoop::push_back(MainLoop::Handler *handler) {
		Handlers::getInstance().push_back(handler);
	}

	void MainLoop::remove(MainLoop::Handler *handler) {
		Handlers::getInstance().remove(handler);
	}

	bool MainLoop::enabled(const MainLoop::Handler *handler) const noexcept {
		return Handlers::getInstance().enabled(handler);
	}

 }
*/
