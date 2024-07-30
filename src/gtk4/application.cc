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
 #include <vector>

 #ifdef HAVE_UDJAT_DBUS
	#include <udjat/tools/dbus/connection.h>
	#include <udjat/tools/dbus/message.h>
 #endif // HAVE_UDJAT_DBUS

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

	void Gtk::Application::help(std::ostream &out) const noexcept {
		Udjat::Application::help(out);
		out << "  --text\tRun in text mode" << endl;
	}

	static bool text_mode() noexcept {

#ifndef _WIN32
		const char *session_type = getenv("XDG_SESSION_TYPE");
		if(session_type) {
			Logger::String{"Detected '",session_type,"' session"}.trace("application");
			return session_type[0] == 0;
		}
#endif // _WIN32

		return false;
	}

	/// @param definitions Path to a single xml file or a folder with xml files.
	int Gtk::Application::run(int argc, char **argv, const char *definitions) {

		debug("---------------------------------Definitions='",definitions,"'");

		if(text_mode() || get_argument(argc,argv,'t',"text") || get_argument(argc,argv,'t',"console") ) {

			debug("Running in console mode");

			if(Udjat::Application::setup(argc, argv, definitions)) {
				return -1;
			}

		} else {

			// Reference:
			//
			// https://gnome.pages.gitlab.gnome.org/gtkmm/

			debug("Running ",String{PRODUCT_ID,".",name().c_str()}.c_str()," in graphic mode");

			g_log_set_default_handler(g_syslog,this);

			if(Udjat::Application::setup(argc, argv, definitions)) {
				return -1;
			}

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

	int Gtk::Application::select(const Dialog &settings, int cancel, const char *button, va_list args) noexcept {

		int rc = -1;

		auto mainloop = Glib::MainLoop::create();

#ifdef USE_GTK_ALERT_DIALOG
		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1AlertDialog.html
		auto dialog = ::Gtk::AlertDialog::create();
		dialog->set_modal();
		dialog->set_message(settings.message());

		auto details = settings.details();
		if(!details.empty()) {
			dialog->set_detail(details);
		}

		if(button) {

			std::vector<Glib::ustring> labels;
			while(button) {
				labels.push_back(button);
				button = va_arg(args, const char *);
			}

			dialog->set_buttons(labels);
			dialog->set_cancel_button(cancel);

		}

		dialog->choose(get_active_window(),[dialog,&mainloop,&rc](Glib::RefPtr<Gio::AsyncResult> result){
			rc = dialog->choose_finish(result);
			debug("Selected button=",rc);
			mainloop->quit();
		});
#else
		::Gtk::MessageDialog dialog{get_active_window(),settings.message(),true,::Gtk::MessageType::INFO,::Gtk::ButtonsType::NONE};
		dialog.set_modal();

		auto details = settings.details();
		if(!details.empty()) {
			dialog.set_secondary_text(details,true);
		}

		if(button) {

			int id = 0;
			while(button) {
				dialog.add_button(button,id++);
				button = va_arg(args, const char *);
			}

		}

		dialog.signal_response().connect([this,mainloop,&rc](int response){
			rc = response;
			mainloop->quit();
		});

		dialog.present();
#endif
		mainloop->run();
		return rc;

	}

	static void reboot(const Dialog &settings) {
#if defined(HAVE_UDJAT_DBUS)
		try {
			// Ask gnome for reboot.
			//
			// http://askubuntu.com/questions/15428/reboot-without-sudoer-privileges
			//
			// Logout: dbus-send --session --type=method_call --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.Logout uint32:1
			//
			// Reboot: dbus-send --session --type=method_call --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.Reboot
			//
			// Shutdown: dbus-send --session --type=method_call --print-reply --dest=org.gnome.SessionManager /org/gnome/SessionManager org.gnome.SessionManager.Shutdown
			//
			DBus::SessionBus{}.call(
				"org.gnome.SessionManager",
				"/org/gnome/SessionManager",
				"org.gnome.SessionManager",
				"Shutdown",
				[](Udjat::DBus::Message &response){
					if(response) {
						Logger::String{"Reboot request sent to gnome session manager"}.info("dialog");
						Gio::Application::get_default()->quit();
					} else {
						Logger::String{"Error '",response.error_message(),"' sending reboot request to gnome"}.error("dialog");
					}
				}
			);
		} catch(const std::exception &e) {
			Logger::String{"Error '",e.what(),"' sending reboot request to gnome"}.warning("dialog");
		}
#endif // HAVE_UDJAT_DBUS
		settings.reboot();
	}

	void Gtk::Application::present(const Dialog &settings, const char *message, const char *details) noexcept {

		struct {
			std::string message;
			std::string details;
		} text;

		if(message && *message) {
			text.message = message;
		} else {
			text.message = settings.message();
		}

		if(details && *details) {
			text.details = details;
		} else {
			text.details = settings.details();
		}

		Glib::signal_idle().connect([text,settings](){

			if(settings.test(Dialog::NonInteractiveReboot)) {
				reboot(settings);
				return 0;
			}

			if(settings.test(Dialog::NonInteractiveQuit)) {
				Gio::Application::get_default()->quit();
				return 0;
			}

			auto dialog = ::Gtk::AlertDialog::create();
			dialog->set_modal();
			dialog->set_message(text.message);

			if(!text.details.empty()) {
				dialog->set_detail(text.details);
			}

			static const struct {
				Dialog::Option option;
				const char *label;
			} buttons[] = {
				{ Dialog::AllowReboot,			N_("_Reboot")			},
				{ Dialog::AllowQuitApplication,	N_("_Quit application")	},
				{ Dialog::AllowCancel,			N_("_Cancel")			},
				{ Dialog::AllowContinue,		N_("C_ontinue")			}
			};

			std::vector<Glib::ustring> labels;
			std::vector<int> values;

			int cancel_button = -1;

			for(int ix = 0; ix < (int) (sizeof(buttons)/sizeof(buttons[0])); ix++) {
				if(settings.test(buttons[ix].option)) {
					labels.push_back(gettext(buttons[ix].label));
					values.push_back(ix);
				}
				if(buttons[ix].option == Dialog::AllowCancel) {
					cancel_button = ix;
				}
			}

			dialog->set_buttons(labels);
			if(cancel_button > 0) {
				dialog->set_cancel_button(cancel_button);
			}

			dialog->choose(get_active_window(),[dialog,&settings,values](Glib::RefPtr<Gio::AsyncResult> result){

				auto rc = values[dialog->choose_finish(result)];
				Logger::String{"User selected option '",buttons[rc].label,"'"}.info("Dialog");

				switch(rc) {
				case 0: // Dialog::AllowReboot
					reboot(settings);
					break;

				case 1: // Dialog::AllowQuitApplication
					Gio::Application::get_default()->quit();
					break;

				case 2: // Dialog::AllowCancel
					break;

				case 3: // Dialog::AllowContinue
					break;

				}

			});

			return 0;
		});

	}

	/*
	int Gtk::Application::failed(const Dialog &settings, bool allow_continue) {

		int rc = -1;
		if(allow_continue) {
			rc = settings.select(settings, 0, _("_Quit application"), _("_Continue"), nullptr);
		} else {
			rc = settings.select(settings, 0, _("_Quit application"), nullptr);
		}
		return 0;

	}
	*/

	::Gtk::Window & Gtk::Application::get_active_window() {
		return *Glib::wrap(gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default())));
	}


 }


