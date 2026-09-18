/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements gtk4 toplevel.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <vector>
 #include <private/application.h>
 #include <iostream>

 #ifdef LOG_DOMAIN
	#undef LOG_DOMAIN
 #endif
 #define LOG_DOMAIN "tui"
 #include <udjat/tools/logger.h>

 using namespace Udjat;
 using namespace std; 

 namespace Reinstall {

	int Application::run_tui() {

		class TextApplication : public Reinstall::Application {
		public:

			TextApplication() {
			}

			int run_interactive() override {

				return -1;
			}

			int run_non_interactive() override {

				return -1;
			}

			bool build(const Udjat::Properties &props) override {
				
				debug("Building group '",props["name"].c_str(),"'");
				

				return true;
			}

		};

		return TextApplication{}.run();
	}

 }
 