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
 #include <udjat/ui/gtk4/application.h>
 #include <udjat/tools/threadpool.h>

 #include <memory>
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
	private:
		struct {
			Glib::RefPtr<Glib::TimeoutSource> source;
			unsigned int idle = (unsigned int) -1;
		} timer;

	public:
		ProgressBar(const char *style) {
			get_style_context()->add_class(style);
			set_hexpand(true);
			set_vexpand(false);
			set_valign(::Gtk::Align::START);
			set_halign(::Gtk::Align::FILL);
			set_ellipsize(Pango::EllipsizeMode::END);

			timer.source = Glib::TimeoutSource::create(100);

			timer.source->connect([this]{

				if(!is_visible()) {
					return true;
				}

				if(timer.idle >= 100) {
					::Gtk::ProgressBar::pulse();
				} else {
					timer.idle++;
				}

				return true;

			});

			timer.source->attach(Glib::MainContext::get_default());

		}

		~ProgressBar() {
			timer.source->destroy();
		}

		inline ProgressBar & operator = (const double fraction) {
			timer.idle = 0;
			Glib::signal_idle().connect([this,fraction](){
				set_fraction(fraction);
				return 0;
			});
			return *this;

		}

		inline void pulse() {
			timer.idle = 1000;
		}

	};

	std::shared_ptr<Dialog::Progress> Gtk::Application::ProgressFactory() {

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Window.html

		class Progress : public Udjat::Dialog::Progress, private ::Gtk::Window {
		private:
			Label message{"dialog-title"}, body{"dialog-subtitle"}, left{"dialog-left-label"}, right{"dialog-right-label",::Gtk::Align::END};
			ProgressBar progress{"dialog-progress-bar"};
			::Gtk::Image icon;

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
				view.set_row_spacing(6);
				view.set_row_homogeneous(false);
				view.set_column_homogeneous(false);
				view.set_margin(12);
				view.get_style_context()->add_class("dialog-contents");

				icon.get_style_context()->add_class("dialog-icon");
				icon.set_hexpand(false);
				icon.set_vexpand(false);

				view.attach(message,1,0,1,1);
				view.attach(body,1,1,1,1);
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
				message = "The message";
				body = "The body";
				progress = .5;
				left = "left";
				right = "right";
#endif // DEBUG

				set_child(view);
				view.show();

			}

			~Progress() {
			}

			void title(const char *title) override {
				string str{title};
				Glib::signal_idle().connect([this,str](){
					set_title(str);
					return 0;
				});
			}

			Dialog::Progress & operator = (const double fraction) override {
				progress = fraction;
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

