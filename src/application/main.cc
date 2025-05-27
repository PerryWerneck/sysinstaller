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
 #include <udjat/tools/string.h>
 #include <reinstall/tools/kernelparameter.h>
 #include <reinstall/tools/repository.h>
 #include <reinstall/tools/writer.h>
 #include <private/console.h>

 #ifdef HAVE_GTKMM
 #include <gtkmm.h>
 #include <private/toplevel.h>
 #endif

 using namespace std;
 using namespace Udjat;

 static bool tui_only() noexcept {

#ifndef _WIN32
	const char *session_type = getenv("XDG_SESSION_TYPE");
	if(session_type) {
		Logger::String{"Detected '",session_type,"' session"}.trace();
		return session_type[0] == 0;
	}
#endif // _WIN32

		return false;
 }

 int main(int argc, char* argv[]) {

	// Check for help options.
	static const Udjat::Application::Option options[] = {
#ifdef HAVE_GTKMM
		{ 't', "tui", _( "Use text user interface" ) },
		{ 'g', "gui", _( "Use graphic user interface" ) },
#endif // HAVE_GTKMM
		{ 'v', "version", _( "Show version information" ) },
		{ 'V', "verbose", _( "Increase verbosity level" ) },
		{ 'q', "quiet", _( "Decrease verbosity level" ) },
		{ 'd', "debug", _( "Enable debug mode" ) },
		{ 'k', "kernel-parameter=n=v", _( "Set kernel parameter 'n' to 'v'" ) },
		{ 'O', "output=img", _( "Write resulting image to file 'img' instead of usb" ) },
		{ 'i', "install=url", _("Set install url, disable slp") },
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

	Logger::redirect();

	// Parse command line options.
	{
		std::string value;

		if(Udjat::Application::pop(argc,argv,'i',"install",value)) {
			Reinstall::Repository::preset("install",value.c_str());	
		}

		if(Udjat::Application::pop(argc,argv,'O',"output",value)) {
			Reinstall::Writer::set_output(value.c_str());
		}

		if(Udjat::Application::pop(argc,argv,'S',"select",value)) {
			auto args = Udjat::String{value.c_str()}.split("/",2);
			if(args.size() != 2) {
				throw std::runtime_error{Logger::Message(_("Unable to select '{}': Invalid path"),value)};
			}
		}

		while(Udjat::Application::pop(argc,argv,'R',"repo",value)) {
			auto args = Udjat::String{value.c_str()}.split("=",2);
			if(args.size() != 2) {
				throw std::runtime_error{Logger::Message(_("Invalid repository argument: {}"),value)};
			}
			Reinstall::Repository::preset(args[0].c_str(),args[1].c_str());
		}

		while(Udjat::Application::pop(argc,argv,'k',"kernel-parameter",value)) {
			Reinstall::KernelParameter::preset(value.c_str());
		}

	}


#ifdef HAVE_GTKMM
	// If the user requested TUI, run as a TUI application.

	if( !Udjat::Application::pop(argc,argv,'g',"gui") && (Udjat::Application::pop(argc,argv,'t',"tui") || tui_only())) {

		// Run as a TUI application.
		return Reinstall::Console{}.run(argc,argv);

	} else {

		// Run as a GUI application.
		return Gtk::Application::create(UDJAT_PRODUCT_DOMAIN ".gtk." PACKAGE_NAME)->make_window_and_run<TopLevel>(1,argv);

	}
#else 
	{
		No GTK, run as TUI application
		return Reinstall::Console{}.run(argc,argv);

	}
#endif // HAVE_GTKMM

	return 0;
 }

