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
  * @brief Application entry point.
  */

 #include <config.h>

 #include <udjat/tools/commandlineparser.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <reinstall/tools/kernelparameter.h>
 #include <reinstall/tools/repository.h>
 #include <reinstall/tools/writer.h>
 #include <reinstall/dialog.h>
 #include <reinstall/application.h>
 #include <private/console.h>
 #include <udjat/tools/quark.h>
 
 #ifdef HAVE_GTKMM
 #include <gtkmm.h>
 #include <private/toplevel.h>
 #endif

 using namespace std;
 using namespace Udjat;

#ifdef HAVE_GTKMM
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
#endif // HAVE_GTKMM

 int main(int argc, char* argv[]) {

	// Check for help options.
	static const Udjat::CommandLineParser::Argument options[] = {
#ifdef HAVE_GTKMM
		{ 't', "tui", _( "Use text user interface" ) },
		{ 'g', "gui", _( "Use graphic user interface" ) },
#endif // HAVE_GTKMM
		{ 'k', "kernel-parameter=n=v", _( "Set kernel parameter 'n' to 'v'" ) },
		{ 'O', "output=img", _( "Write resulting image to file 'img' instead of usb" ) },
		{ 'i', "install=url", _("Set install url, disable slp") },
		{ 'r', "repo=r=u", _("Set url for repository 'r' to 'u', disable slp") },
		{ 'y', "non-interactive", _("Non interactive") },
		{ 'Q', "quit", _("Quit after processing") },
		{ 'R', "reboot", _("Reboot on success") },
		{ 'S', "select=option", _( "Auto select option" ) },
		{ 'T', "target=target", _( "Auto select target" ) },
		{ }
	};

	bool non_interactive = false;
	{
		const char *ptr = strrchr(argv[0],'/');
		if(ptr) {
			ptr++;
			non_interactive = (strcmp(ptr,"reinstall-enable") == 0 || strcmp(ptr,"reinstall-system") == 0);
		}
	}

	// Initialize the application.
	Quark::init();

#ifdef DEBUG
	Udjat::Config::allow_user_homedir(true);
	Logger::verbosity(9);
	Logger::console(true);
#endif

	if(Udjat::CommandLineParser::options(argc,argv,options)) {
		return 0;
	}

	Logger::redirect();

	// Parse command line options.
	{
		CommandLineParser options(argc,argv);
		String value;

		if(options.get_argument(argc,argv,'i',"install",value)) {
			Reinstall::Repository::preset("install",value.c_str());	
		}

		if(options.get_argument(argc,argv,'O',"output",value)) {
			Reinstall::Writer::set_output(value.c_str());
		}

		if(options.get_argument(argc,argv,'S',"select",value)) {
			Reinstall::Action::preset(value.c_str());
		}

		if(options.get_argument(argc,argv,'S',"select",value)) {
			Reinstall::Action::preset(value.c_str());
		}

		if(options.get_argument(argc,argv,'T',"target",value)) {

			auto targets = value.split(",",2);

			if(targets.empty() || targets[0].empty()) {
				throw std::runtime_error{Logger::Message(_("Invalid target '{}', please check your configuration"),value)};
			}

			//
			// Get TARGET URL
			//
			URL target{Config::Value<string>{"install-targets",targets[0].c_str()}.c_str()};

			if(target.empty()) {
				target = Config::Value<string>{"install-targets","default"}.c_str();
			}

			if(target.empty()) {
				throw std::runtime_error{Logger::Message(_("No target found for '{}', please check your configuration"),targets[0])};
			}

			target.expand([&targets](const char *name, std::string &val) -> bool{

				if(strcasecmp(name,"hostname") == 0 || strcasecmp(name,"ip") == 0 || strcasecmp(name,"server") == 0) {
					if(targets.size() < 2) {
						throw std::runtime_error{"This target requires a second argument, please check your configuration"};
					}
					val = targets[1].c_str();
					return true;

				} else if(strcasecmp(name,"target") == 0) {
					val = targets[0].c_str();
					return true;
				}
				
				return false;
			});

			if(targets.size() > 1) {
				target.hostname(targets[1].c_str());
			}

			Logger::String{"Using non interactive mode with target '",target.c_str(),"'"}.info();

			Reinstall::Dialog::preset(Reinstall::Dialog::NonInteractive);
			Reinstall::Repository::preset("install",target.c_str());	

		}

		if(options.has_argument(argc,argv,'R',"reboot")) {
			Reinstall::Dialog::preset(Reinstall::Dialog::NonInteractiveReboot);
		}

		if(options.has_argument(argc,argv,'y',"non-interactive") || non_interactive) {
			Reinstall::Dialog::preset(Reinstall::Dialog::NonInteractive);
		}

		while(options.get_argument(argc,argv,'r',"repo",value)) {
			auto args = Udjat::String{value.c_str()}.split("=",2);
			if(args.size() != 2) {
				throw std::runtime_error{Logger::Message(_("Invalid repository argument: {}"),value)};
			}
			Reinstall::Repository::preset(args[0].c_str(),args[1].c_str());
		}

		while(options.get_argument(argc,argv,'k',"kernel-parameter",value)) {
			Reinstall::KernelParameter::preset(value.c_str());
		}

	}


#ifdef HAVE_GTKMM
	// If the user requested TUI, run as a TUI application.

	if( !CommandLineParser::has_argument(argc,argv,'g',"gui") && (CommandLineParser::has_argument(argc,argv,'t',"tui") || tui_only())) {

		// Run as a TUI application.
		return Reinstall::Console{}.run(argc,argv);

	} else {

		// Run as a GUI application.
		if(Reinstall::Dialog::has_preset(Reinstall::Dialog::NonInteractive)) {

			return Gtk::Application::create(G_STRINGIFY(PACKAGE_DOMAIN) "." PACKAGE_NAME)->make_window_and_run<NonInteractiveWindow>(1,argv);

		} else {

			return Gtk::Application::create(G_STRINGIFY(PACKAGE_DOMAIN) "." PACKAGE_NAME)->make_window_and_run<InteractiveWindow>(1,argv);
		
		}

	}
#else 
	{
		// No GTK, run as TUI application
		return Reinstall::Console{}.run(argc,argv);

	}
#endif // HAVE_GTKMM

	return 0;
 }

