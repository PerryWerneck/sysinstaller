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
  * @brief Implements gtk4 application..
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>

 #include <udjat/ui/dialog.h>
 #include <udjat/ui/gtk4/application.h>
 #include <udjat/ui/gtk4/mainloop.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/threadpool.h>

 #include <gtkmm.h>

 using namespace std;

 namespace Udjat {

	Gtk::Application::Application() {
		static Udjat::Gtk::MainLoop mainloop;
		mainloop.active();

	}

	Gtk::Application::~Application() {
	}

	static const char * get_argument(int argc, char **argv, const char shortname, const char *longname) {

		size_t szlong = strlen(longname);

		argc--;
		for(int ix = 1; ix <= argc; ix++) {

			const char *arg = argv[ix];

			if(arg[0] == '-' && arg[1] == '-' && arg[2] && strncasecmp(arg+2,longname,szlong) == 0) {

				if(arg[2+szlong] == '=') {
					return arg + 2 + szlong;
				} else if(!arg[2+szlong]) {
					return "1";
				}

			}

			if(arg[0] == '-' && arg[1] == shortname && arg[2] == 0) {
				if(ix < argc && argv[ix+1][0] != '-' ) {
					return argv[ix+1];
				}
				return "1";
			}

		}

		return nullptr;
	}

	static void g_syslog(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer G_GNUC_UNUSED(user_data)) {

		static const struct Type {
			GLogLevelFlags			level;
			Udjat::Logger::Level	lvl;
		} types[] =
		{
			{ G_LOG_FLAG_RECURSION,	Udjat::Logger::Info			},
			{ G_LOG_FLAG_FATAL,		Udjat::Logger::Error		},

			// GLib log levels
			{ G_LOG_LEVEL_ERROR,	Udjat::Logger::Error		},
			{ G_LOG_LEVEL_CRITICAL,	Udjat::Logger::Error		},
			{ G_LOG_LEVEL_WARNING,	Udjat::Logger::Warning		},
			{ G_LOG_LEVEL_MESSAGE,	Udjat::Logger::Info			},
			{ G_LOG_LEVEL_INFO,		Udjat::Logger::Info			},
			{ G_LOG_LEVEL_DEBUG,	Udjat::Logger::Debug		},
		};

		for(size_t ix=0; ix < G_N_ELEMENTS(types);ix++) {
			if(types[ix].level == level) {
				Udjat::Logger::String{message}.write(types[ix].lvl,domain ? domain : "gtk");
				return;
			}
		}

		Udjat::Logger::String{message}.error(domain ? domain : "gtk");

	}

	/// @param definitions Path to a single xml file or a folder with xml files.
	int Gtk::Application::run(int argc, char **argv, const char *definitions) {

		debug("---------------------------------Definitions='",definitions,"'");

		if(get_argument(argc,argv,'t',"text") || get_argument(argc,argv,'t',"console") ) {

			debug("Running in console mode");


		} else {

			// Reference:
			//
			// https://gnome.pages.gitlab.gnome.org/gtkmm/

			debug("Running ",String{PRODUCT_ID,".",name().c_str()}.c_str()," in graphic mode");

			g_log_set_default_handler(g_syslog,NULL);

			Glib::RefPtr<::Gtk::Application> app = ::Gtk::Application::create(String{PRODUCT_ID,".",name().c_str()}.as_quark());
			::Gtk::Application::set_default(app);

			app->signal_startup().connect([app,definitions,this](){

				debug("Starting up");
#ifdef DEBUG
				std::string iconpath{"./icons"};
#else
				Udjat::Application::DataDir iconpath{"icons"};
#endif // DEBUG

				Logger::String{"Searching '",iconpath.c_str(),"' for customized icons"}.trace("gtk");

				// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1IconTheme.html
				::Gtk::IconTheme::get_for_display(Gdk::Display::get_default())->add_search_path(iconpath);

				app->mark_busy();
				try {

					this->startup(app,definitions);

				} catch(const std::exception &e) {

					error() << e.what() << endl;
					app->quit();

				} catch(...) {

					error() << "Unexpected error starting GUI" << endl;
					app->quit();

				}
				app->unmark_busy();

			});

			app->signal_activate().connect([app,definitions,this]{

				debug("Activating");

				app->mark_busy();
				try {

					this->activate(app,definitions);

				} catch(const std::exception &e) {

					error() << e.what() << endl;
					app->quit();

				} catch(...) {

					error() << "Unexpected error activating GUI" << endl;
					app->quit();

				}
				app->unmark_busy();

			});

			app->signal_shutdown().connect([app,definitions,this]{

				debug("Shutting down");

				app->mark_busy();
				try {

					this->shutdown(app,definitions);

				} catch(const std::exception &e) {

					error() << e.what() << endl;

				} catch(...) {

					error() << "Unexpected error deactivating GUI" << endl;

				}
				app->unmark_busy();

			});

			return app->run(1,argv);

		}

		return -1;
	}

	void Gtk::Application::activate(Glib::RefPtr<::Gtk::Application>, const char *) {

		debug("-------------------------> Activate");

	}

	void Gtk::Application::startup(Glib::RefPtr<::Gtk::Application>, const char *definitions) {

		debug("-------------------------> Starting up");
		init(definitions);

	}

	void Gtk::Application::shutdown(Glib::RefPtr<::Gtk::Application>, const char *definitions) {

		debug("-------------------------> Shutting down");
		deinit(definitions);

	}

	int Gtk::Application::run(Udjat::Dialog *, const std::function<int()> &task) noexcept {

		int rc = -1;

		auto mainloop = Glib::MainLoop::create();

		Udjat::ThreadPool::getInstance().push([&task,&rc,mainloop](){

			try {

				rc = task();

			} catch(const std::exception &e) {

				// TODO: Show error popup

				rc = -1;
				Logger::String{e.what()}.error("gtk");

			} catch(...) {

				// TODO: Show error popup

				rc = -1;
				Logger::String{"Unexpected error running background task"}.error("gtk");

			}

			Glib::signal_idle().connect([mainloop](){
				mainloop->quit();
				return 0;
			});

		});

		mainloop->run();

		return rc;

	}


 }


