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
  * @brief Implements gtk4 popup.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/ui/progress.h>
 #include <reinstall/ui/progress.h>
 #include <reinstall/ui/application.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/string.h>

 #include <memory>
 #include <glib/gi18n-lib.h>
 #include <gtkmm.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	class Label : public ::Gtk::Label {
	public:
		Label(const char *style, ::Gtk::Align align = ::Gtk::Align::START, Pango::EllipsizeMode mode = Pango::EllipsizeMode::START) {
			get_style_context()->add_class(style);
			set_hexpand(true);
			set_vexpand(false);
			set_valign(::Gtk::Align::START);
			set_halign(align);
			set_single_line_mode();
			set_ellipsize(mode);
		}

		inline Label & operator = (const char *value) {
			string str{value};
			Glib::signal_idle().connect([this,str](){
				set_text(str);
				return 0;
			});
			return *this;
		}

	};

	class ProgressBar : public Udjat::Dialog::Progress, public ::Gtk::Grid {
	private:
		::Gtk::ProgressBar bar;
		Label left{"dialog-left-label"};
		Label right{"dialog-right-label",::Gtk::Align::END};

		Glib::RefPtr<Glib::TimeoutSource> timer;
		unsigned int idle = (unsigned int) -1;

		bool changed = false;
		uint64_t current = 0;
		uint64_t total = 0;

	public:

		ProgressBar() {

			// Setup grid
			set_hexpand(true);
			set_vexpand(false);
			set_column_spacing(3);
			set_row_spacing(3);
			get_style_context()->add_class("dialog-footer");
			set_valign(::Gtk::Align::END);
		
			// Setup bar
			bar.get_style_context()->add_class("dialog-progress-bar");
			bar.set_hexpand(true);
			bar.set_vexpand(false);
			bar.set_valign(::Gtk::Align::START);
			bar.set_halign(::Gtk::Align::FILL);
			bar.set_ellipsize(Pango::EllipsizeMode::START);

			// Add widgets to grid
			attach(bar,0,0,2,1);
			attach(left,0,1,1,1);
			attach(right,1,1,1,1);

#ifdef DEBUG
			left = "left";
			right = "right";
#endif // DEBUG

			timer = Glib::TimeoutSource::create(100);

			timer->connect([this]{

				if(changed) {

					idle = 0;
					changed = false;

					if(total) {
						double fraction =  ((double) current) / ((double) total);
						if(fraction > 1.0) {
							bar.set_fraction(1.0);
						} else {
							bar.set_fraction(fraction);
						}

						right = Logger::Message{_("{} of {}"),
									String{""}.set_byte((unsigned long long) current).c_str(),
									String{""}.set_byte((unsigned long long) total).c_str()
								}.c_str();

					} else {
						idle = 1000;
						right = "";
					}
		
				} else if(idle >= 100) {

					bar.pulse();

				} else {

					idle++;
			//		debug(idle);

				}

				return true;

			});

			timer->attach(Glib::MainContext::get_default());

		}

		virtual ~ProgressBar() {
			timer->destroy();
		}

		Udjat::Dialog::Progress & item(const short current = 0, const short total = 0) override {
			Glib::signal_idle().connect([this,current,total](){
				if(total) {
					left.set_text(Logger::Message{_("{} of {}"), current, total}.c_str());
				} else {
					left.set_text("");
				}
				return 0;
			});
			return *this;
		}

		Udjat::Dialog::Progress & set(uint64_t current, uint64_t total, bool) override {
			this->current = current;
			this->total = total;
			changed = true;
			return *this;
		}

		Udjat::Dialog::Progress & set(const char *url) override {
			string u{url};
			Glib::signal_idle().connect([this,u](){
				bar.set_text(u);
				return 0;
			});
			return *this;
		}
	
	};

	std::shared_ptr<Reinstall::Dialog::Progress> Reinstall::Application::ProgressFactory() {

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Window.html

		class Progress : public Reinstall::Dialog::Progress, private ::Gtk::Window {
		private:
			Label main{"dialog-title"}, subtitle{"dialog-subtitle"};

			shared_ptr<ProgressBar> bar{make_shared<ProgressBar>()};

			::Gtk::Image icon;

			struct {
				string label;
				uint8_t changed = 0;
				bool valid = false;
				float fraction;
			} values;

		public:
			Progress() {
				gtk_window_set_transient_for(
					GTK_WINDOW(gobj()),
					gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
				);

				set_modal(true);
				set_deletable(false);
				set_resizable(false);

				set_decorated(false);
				set_default_size(500,-1);

				get_style_context()->add_class("dialog-progress");

				::Gtk::Grid view;
				view.set_hexpand(true);
				view.set_vexpand(true);
				view.set_column_spacing(3);
				view.set_row_spacing(3);
				view.set_row_homogeneous(false);
				view.set_column_homogeneous(false);
				view.set_margin(12);
				view.get_style_context()->add_class("dialog-contents");

				icon.get_style_context()->add_class("dialog-icon");
				icon.set_hexpand(false);
				icon.set_vexpand(false);
				// icon.set_icon_size(::Gtk::IconSize::LARGE);
				icon.set_pixel_size(45);
				icon.set_from_icon_name("logo");

				view.attach(main,1,0,1,1);
				view.attach(subtitle,1,1,1,1);
				view.attach(icon,0,0,1,2);
				view.attach(*bar,0,2,2,1);

#ifdef DEBUG
				main = "The message";
				subtitle = "The body";
#endif // DEBUG

				set_child(view);
				view.show();


			}

			~Progress() {
				debug("-----------------> GTK4 progress dialog was destroyed");
			}

			Dialog::Progress & title(const char *title) override {
				string str{title};
				Glib::signal_idle().connect([this,str](){
					set_title(str);
					return 0;
				});
				return *this;
			}

			Dialog::Progress & show() override {
				Glib::signal_idle().connect([this](){
					set_visible(true);
					return 0;
				});
				return *this;
			}

			Dialog::Progress & hide() override {
				Glib::signal_idle().connect([this](){
					set_visible(false);
					return 0;
				});
				return *this;
			}

			Dialog::Progress & message(const char *text) override {
				main = text;
				return *this;
			}

			Dialog::Progress & body(const char *text) override {
				subtitle = text;
				return *this;
			}

			Dialog::Progress & icon_name(const char *name) override {
				string str{name};
				Glib::signal_idle().connect([this,str](){
					icon.set_from_icon_name(str);
					return 0;
				});
				return *this;
			}

			Dialog::Progress & item(const size_t current, const size_t total) override {
				bar->item(current,total);
				return *this;
			}

			Dialog::Progress & url(const char *url) override {
				bar->set(url);
				return *this;
			}

			Dialog::Progress & file_sizes(const uint64_t current, const uint64_t total) {
				bar->set(current,total,true);
				return *this;
			}
 
			int run(const std::function<int(Dialog::Progress &progress)> &task) noexcept override {

				present();

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

		return make_shared<Progress>();

	}


 }

