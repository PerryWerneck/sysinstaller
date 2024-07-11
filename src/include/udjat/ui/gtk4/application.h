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
  * @brief Implements grafic application.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 #include <gtkmm.h>
 #include <udjat/ui/dialog.h>

 namespace Udjat {

	namespace Gtk {

		class UDJAT_API Application : public Udjat::Application, private Udjat::Dialog::Controller {
		public:
			Application();
			virtual ~Application();

			/// @param definitions Path to a single xml file or a folder with xml files.
			int run(int argc, char **argv, const char *definitions = nullptr) override;

			static ::Gtk::Window & get_active_window();

		protected:

			typedef Udjat::Gtk::Application super;

			/// @brief Gui is starting
			virtual void startup(Glib::RefPtr<::Gtk::Application> app, const char *definitions);

			/// @brief GUI was activated.
			virtual void activate(Glib::RefPtr<::Gtk::Application> app, const char *definitions);

			/// @brief GUI is shutting down.
			virtual void shutdown(Glib::RefPtr<::Gtk::Application> app, const char *definitions);

			// Udjat::Dialog::Controller
			std::shared_ptr<Udjat::Dialog::Popup> PopupFactory() override;
			std::shared_ptr<Udjat::Dialog::Progress> ProgressFactory() override;
			int select(const Dialog &dialog, int cancel, const char *button, va_list args) noexcept override;
			void present(const Dialog &dialog, const char *message, const char *details) override;

		};

	}

 }
