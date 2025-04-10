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
  * @brief Declares gtk4 mainloop binding for libudjat.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>

 namespace Udjat {

	namespace Gtk {

		class UDJAT_API MainLoop : private Udjat::MainLoop {
		public:
			MainLoop();
			virtual ~MainLoop();

			void wakeup() noexcept override;

			// Mainloop methods
			bool enabled(const Timer *timer) const noexcept override;

			/// @brief Is Handler enabled?
			bool enabled(const Handler *handler) const noexcept override;

			void push_back(MainLoop::Timer *timer) override;
			void remove(MainLoop::Timer *timer) override;

			void push_back(MainLoop::Handler *handler) override;
			void remove(MainLoop::Handler *handler) override;

			/// @brief Run mainloop.
			int run() override;

			/// @brief Is the mainloop active?
			bool active() const noexcept override;

			void post(Message *message) noexcept override;

			inline operator bool() const noexcept {
				return active();
			}

			/// @brief Quit mainloop.
			void quit() override;


		};

	}

 }

