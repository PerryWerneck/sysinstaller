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
  * @brief Application entry point.
  */

 #include <config.h>

 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/version.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/tools/kernelparameter.h>

 #ifdef HAVE_GTKMM
 #include <gtkmm.h>
 #include <private/toplevel.h>
 #endif

 using namespace std;
 using namespace Udjat;

 int main(int argc, char* argv[]) {

#ifdef DEBUG 
	Config::allow_user_homedir(true);
	Logger::verbosity(9);
	Logger::console(true);
#endif // DEBUG

	// Check for help options.
	static const Udjat::Application::Option options[] = {
		{ 't', "text", _( "Run in text mode" ) },
		{ 'O', "output=img", _( "Write resulting image to file 'img' instead of usb" ) },
		{ 'R', "repo=r=u", _("Set url for repository 'r' to 'u', disable slp") },
		{ 'K', "kparm=n=v", _("Set kernel parameter 'n' to 'v' on boot image") },
		{ 'Q', "quit", _("Quit after processing") },
		{ 'R', "reboot", _("Reboot when success") },
		{ 'S', "select=[option]", _( "Auto select option" ) },
		{ 'h', "help", _( "Show this help" ) },
		{ }
	};

	if(Udjat::Application::options(argc,argv,options)) {
		return 0;
	}

#ifdef DEBUG
	Udjat::Config::allow_user_homedir(true);
	Logger::verbosity(9);
	Logger::console(true);
#endif
  
	debug("argc=",argc);

	// Load kernel parameters.
	{
		string value;
		while(Udjat::Application::pop(argc,argv,'k',"kernel-parameter",value)) {
			Reinstall::KernelParameter::preset(value.c_str());
		}
	}


#ifdef HAVE_GTKMM
	{
		// Run as a GUI application.
		return Gtk::Application::create(UDJAT_PRODUCT_DOMAIN "." PACKAGE_NAME)->make_window_and_run<TopLevel>(1,argv);

	}
#endif // HAVE_GTKMM

	return 0;
 }

