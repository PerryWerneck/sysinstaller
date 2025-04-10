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
 #include <reinstall/ui/application.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/string.h>

 #include <memory>
 #include <glib/gi18n-lib.h>
 #include <gtkmm.h>

 using namespace std;

 namespace Udjat {

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

	class ProgressBar : public ::Gtk::ProgressBar {
	public:
		ProgressBar(const char *style) {
			get_style_context()->add_class(style);
			set_hexpand(true);
			set_vexpand(false);
			set_valign(::Gtk::Align::START);
			set_halign(::Gtk::Align::FILL);
			set_ellipsize(Pango::EllipsizeMode::START);
		}

		~ProgressBar() {
		}

	};

	std::shared_ptr<Dialog::Progress> Gtk::Application::ProgressFactory() {

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Window.html

		class Progress : public Udjat::Dialog::Progress, private ::Gtk::Window {
		private:
			Label main{"dialog-title"}, subtitle{"dialog-subtitle"}, left{"dialog-left-label"}, right{"dialog-right-label",::Gtk::Align::END};
			ProgressBar progress{"dialog-progress-bar"};
			::Gtk::Image icon;

			Glib::RefPtr<Glib::TimeoutSource> timer;

			unsigned int idle = (unsigned int) -1;

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
				view.attach(progress,0,2,2,1);

				::Gtk::Box footer{::Gtk::Orientation::HORIZONTAL};
				footer.get_style_context()->add_class("dialog-footer");
				footer.set_homogeneous(true);
				footer.set_hexpand(true);
				footer.set_vexpand(false);
				footer.set_valign(::Gtk::Align::END);
				footer.append(left);
				footer.append(right);
				view.attach(footer,0,3,2,1);

#ifdef DEBUG
				main = "The message";
				subtitle = "The body";
				left = "left";
				right = "right";
#endif // DEBUG

				set_child(view);
				view.show();

				timer = Glib::TimeoutSource::create(100);

				timer->connect([this]{

					if(values.changed) {

						idle = 0;
						if(values.valid) {
							progress.set_fraction(values.fraction);
						}
						if(values.changed &2) {
							right.set_text(values.label);
						}
						values.changed = 0;

					} else if(idle >= 100 || !values.valid) {

						progress.pulse();

					} else {

						idle++;
//						debug(idle);

					}

					return true;

				});

				timer->attach(Glib::MainContext::get_default());

			}

			~Progress() {
				debug("-----------------> GTK4 progress dialog was destroyed");
				timer->destroy();
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

			Dialog::Progress & operator = (const double fraction) override {
				values.fraction = fraction;
				values.valid = true;
				values.changed = true;
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
				Glib::signal_idle().connect([this,current,total](){
					if(total) {
						left.set_text(Logger::Message{_("{} of {}"),current,total});
					} else {
						left.set_text("");
					}
					return 0;
				});
				return *this;
			}

			Dialog::Progress & url(const char *url) override {
				debug("-------------> url=",url);
				string str{url};
				Glib::signal_idle().connect([this,str](){
					progress.pulse();
					progress.set_text(str);
					progress.set_show_text();
					values.valid = false;
					values.label.clear();
					right.set_text("");
					return 0;
				});
				return *this;
			}

			Dialog::Progress & file_sizes(const uint64_t current, const uint64_t total) {

				string label;

				if(total) {
					values.valid = true;
					values.fraction = ((float) current) / ((float) total);

					if(current) {
						label = Logger::Message{_("{} of {}"),
									String{""}.set_byte((unsigned long long) current).c_str(),
									String{""}.set_byte((unsigned long long) total).c_str()
								};
					}

				} else {
					values.valid = false;
				}

				if(strcmp(label.c_str(),values.label.c_str())) {
					values.changed |= 2;
					values.label = label;
				}

				values.changed |= 1;
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

