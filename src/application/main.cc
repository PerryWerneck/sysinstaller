/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Application entry point.
  */

 #ifdef HAVE_GTKMM
	#include <gtkmm.h>
 #endif
 
 #include <config.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/argumentparser.h>
 #include <udjat/tools/configuration.h>

 using namespace Udjat;

 int main(int argc, char **argv) {

#ifdef HAVE_GTKMM
	enum {
		TEXT_MODE,
		GRAPHICAL_MODE,
	} mode = GRAPHICAL_MODE;
#else
	enum {
		TEXT_MODE,
	} mode = TEXT_MODE;
#endif // HAVE_GTKMM

#ifdef DEBUG
	Udjat::Config::allow_user_homedir(true);
	Logger::verbosity(9);
	Logger::console(true);
#endif


#ifdef HAVE_GTKMM
	if(mode == GRAPHICAL_MODE) {
		Logger::String{"starting graphical mode"}.trace();
		return 0;
	}
#endif // HAVE_GTKMM

	Logger::String{"Starting text mode"}.trace();
	

	return 0;
 }


