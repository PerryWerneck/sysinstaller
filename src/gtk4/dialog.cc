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
  * @brief Implements gtk4 dialogs.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <reinstall/dialog.h>
 #include <private/toplevel.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <memory>

 using namespace Udjat;
 using namespace std;
 using namespace Gtk;

 std::shared_ptr<Reinstall::Dialog> TopLevel::DialogFactory(const char *name, const Udjat::XML::Node &node) {

	class Popup : public MessageDialog {
	public:

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Dialog.html

		Popup(const Reinstall::Dialog &defs) : MessageDialog{""} {
			gtk_window_set_transient_for(
				GTK_WINDOW(gobj()),
				gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
			);
			set_modal(true);
		}

		~Popup() {
		}

	};

	class Dialog : public Reinstall::Dialog {
	private:

		enum Responses {
			DIALOG_RESPONSE_NONE,
			DIALOG_RESPONSE_YES,
			DIALOG_RESPONSE_NO,
			DIALOG_RESPONSE_QUIT,
			DIALOG_RESPONSE_REBOOT,
			DIALOG_RESPONSE_CANCEL,
			DIALOG_RESPONSE_CONTINUE,
		};

	public:
		Dialog(const Udjat::XML::Node &node, const Option option) : Reinstall::Dialog{node,option} {
		}

		virtual ~Dialog() {

		}

		void setup(MessageDialog &window) const {

			if(title && *title) {
				window.set_message(title);
			}

			if(options & AllowQuitApplication) {
				window.add_button(_("_Quit application"),DIALOG_RESPONSE_QUIT);
			}

			if(options & AllowReboot) {
				window.add_button(_("_Reboot"),DIALOG_RESPONSE_REBOOT);
			}
			
			if(options & AllowCancel) {
				window.add_button(_("C_ancel"),DIALOG_RESPONSE_CANCEL);
			}

			if(options & AllowContinue) {
				window.add_button(_("_Continue"),DIALOG_RESPONSE_CANCEL);
			}
			
		}

		/// @brief Ask for confirmation.
		bool ask() const noexcept override {

			auto mainloop = Glib::MainLoop::create();

			// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1MessageDialog.html
			MessageDialog dialog{
				(message && *message) ? message : _("Do you want to continue?"),
				true,
				MessageType::QUESTION,
				ButtonsType::YES_NO,
				true
			};

			if(title && *title) {
				dialog.set_title(title);
			}

			gtk_window_set_transient_for(
				GTK_WINDOW(dialog.gobj()),
				gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
			);

			if(details && *details) {
				dialog.set_secondary_text(details);
			}

			if(destructive) {
				// https://gnome.pages.gitlab.gnome.org/libadwaita/doc/main/style-classes.html
				auto button = dialog.get_widget_for_response(ResponseType::YES);
				button->get_style_context()->add_class("destructive-action");
			}

			dialog.set_default_response(ResponseType::NO);

			bool rc = false;
			dialog.signal_response().connect([mainloop,&rc](int response){
				Logger::String{"Dialog response: ",response}.trace("gtk");
				rc = (response == ResponseType::YES);
				mainloop->quit();
			});

			dialog.present();
			mainloop->run();

			return rc;
		};
		
		/// @brief Show the dialog without any message.
		void present() const noexcept {

		}

		/// @brief Show the dialog with an error message.
		/// @param e The exception to show.
		virtual void present(const std::exception &e) const noexcept {

		}

	};

	/*
	static const struct {
		Dialog::Option value;
		const char * attrname;
	} opts[] {
		{ Dialog::AllowQuitApplication,	"quit-button" 	},
		{ Dialog::AllowReboot,			"reboot-button"	},
	};

	Dialog::Option option = Dialog::None;
	String cfg{"dialog-",name};

	for(const auto &opt : opts) {
		if(Config::Value<bool>(cfg.c_str(),opt.attrname,false)) {
			option = (Dialog::Option) (option|opt.value);
		}
	}
	*/

	return make_shared<Dialog>(node,Dialog::None);
} 


/*
 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <reinstall/ui/popup.h>
 #include <reinstall/ui/application.h>
 #include <udjat/tools/threadpool.h>

 #include <memory>
 #include <gtkmm.h>

 #ifdef USE_GTK_ALERT_DIALOG
	#include <gtkmm/alertdialog.h>
 #endif // USE_GTK_ALERT_DIALOG

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	std::shared_ptr<Reinstall::Dialog::Popup> Application::PopupFactory() {

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1AlertDialog.html

#ifdef USE_GTK_ALERT_DIALOG
		class Popup : public Reinstall::Dialog::Popup, private ::Gtk::AlertDialog {
		public:
			Popup() {
				gtk_window_set_transient_for(
					GTK_WINDOW(gobj()),
					gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
				);
				set_modal(true);
			}

			~Popup() {
			}

			Popup & message(const char *message) override {
				string str{message};
				Glib::signal_idle().connect([this,str](){
					set_message(str);
					return 0;
				});
				return *this;
			}

			Popup & detail(const char *text) override {
				string str{text};
				Glib::signal_idle().connect([this,str](){
					set_detail(str);
					return 0;
				});
				return *this;
			}

			int run(const std::function<int(Dialog::Popup &popup)> &task) noexcept override {

				int rc = -1;
				auto mainloop = Glib::MainLoop::create();

				Udjat::ThreadPool::getInstance().push([this,&task,&rc,mainloop](){

					try {

						rc = task(*this);

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

		};

#else
		class Popup : public Reinstall::Dialog::Popup, private ::Gtk::MessageDialog {
		public:
			Popup() : ::Gtk::MessageDialog{""} {
				gtk_window_set_transient_for(
					GTK_WINDOW(gobj()),
					gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
				);
				set_modal(true);
			}

			~Popup() {
			}

			Popup & message(const char *message) override {
				string str{message};
				Glib::signal_idle().connect([this,str](){
					set_message(str,true);
					return 0;
				});
				return *this;
			}

			Popup & detail(const char *text) override {
				string str{text};
				Glib::signal_idle().connect([this,str](){
					set_secondary_text(str,true);
					return 0;
				});
				return *this;
			}

			int run(const std::function<int(Reinstall::Dialog::Popup &popup)> &task) noexcept override {

				int rc = -1;
				auto mainloop = Glib::MainLoop::create();

				Udjat::ThreadPool::getInstance().push([this,&task,&rc,mainloop](){

					int rc = -1;

					try {

						rc = task(*this);

					} catch(const std::exception &e) {

						// TODO: Show error popup

						rc = -1;
						Logger::String{e.what()}.error("gtk");

					} catch(...) {

						// TODO: Show error popup

						rc = -1;
						Logger::String{"Unexpected error running background task"}.error("gtk");

					}

					Glib::signal_idle().connect([this,&rc,mainloop](){
						mainloop->quit();
						return 0;
					});

				});

				mainloop->run();
				return rc;

			}

		};
#endif // USE_GTK_ALERT_DIALOG

		return make_shared<Popup>();
	}


 }

*/
