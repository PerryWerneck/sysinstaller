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
  * @brief Implements abstract action.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/action.h>
 #include <reinstall/dialog.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <reinstall/dialog.h>
 #include <udjat/tools/intl.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Action::Action(const Udjat::XML::Node &node) 
		: NamedObject{node} {
		
		confirmation = Dialog::Factory("confirmation",node,_("Do you confirm?"));
		success = Dialog::Factory("success",node,_("Operation complete"),Dialog::AllowQuitContinue);
		failed = Dialog::Factory("failed",node,_("Operation failed"),Dialog::AllowQuitContinue);

	}

	Action::~Action() {

	}

	/// @brief Activate the action, called on selected action when the 'apply' button is pressed.
	void Action::activate() {
		throw logic_error(_("The selected action cant be activated"));
	}

	/// @brief Test if the action is valid and can be activated.
	bool Action::initialize() {
		return true;
	}

 }
 
/*
 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/action.h>
 #include <reinstall/group.h>
 #include <reinstall/ui/progress.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/intl.h>

 #include <stdexcept>
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <fstab.h>
 #include <cstdio>
 #include <mntent.h>
 #include <limits.h>
 #include <stdlib.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Action::Action(const Udjat::Abstract::Object &object, const Udjat::XML::Node &node)
		: NamedObject{node},
		 args{node}, confirmation{"confirmation",Dialog::Option::None,node},
		 success{"success",Dialog::Option::AllowQuitContinue,node},
		 failed{"failed",Dialog::Option::AllowQuitContinue,node} {

		args.dialog_title = Quark{object["title"]}.c_str();

		debug("Building action '",name(),"' on group '",object.name(),"'");

		if(!confirmation) {
			Logger::String{"Confirmation dialog is not defined, disabling it"}.trace(name());
		}

		if(!success) {
			Logger::String{"Success dialog is not defined, disabling it"}.trace(name());
		}

		if(!failed) {
			Logger::String{"Failed dialog is not defined, disabling it"}.trace(name());
		}

	}

	Action::~Action() {
		debug("Destroying action '",name(),"'");
	}

	int Action::activate(Reinstall::Dialog::Progress &) {
		throw logic_error(_("The selected action cant be activated"));
	}

	bool Action::initialize() {
		return true;
	}

	void Action::activate() {

		if(confirmation && confirmation.select(0,_("Cancel"),_("Continue"),nullptr) != 1) {
			info() << "Operation cancelled by user" << endl;
			return;
		}

		//
		// Run action
		//
		auto dialog = Reinstall::Dialog::Progress::Factory();

		info() << "Starting activity" << endl;

		dialog->title(args.dialog_title);

		debug("Setting title");
		if(args.title && *args.title) {
			dialog->message(args.title);
		} else {
			Logger::String{"Empty dialog title"}.warning(name());
		}

		dialog->body(_("Initializing"));

		debug("Setting icon name");
		if(args.icon_name && *args.icon_name) {
			dialog->icon_name(args.icon_name);
		}

		struct {
			string message;
			string details;
		} popup;

		debug("Running background thread");
		auto rc = dialog->run([&](Reinstall::Dialog::Progress &progress){

			try {

				debug("Calling action");
				activate(progress);
				debug("--------------------------------------------");
				return 0;

			} catch(const Udjat::HTTP::Exception &e) {

				debug("Udjat::HTTP::exception");
				popup.message = e.title();

				const char *msg = e.body();
				if(msg && *msg) {

					Logger::String{e.body()}.error(name());
					popup.details = e.body();

				} else {

					int code = e.code();
					Logger::String message{"HTTP error ",code};
					message.error(name());

					if(code == 404) {
						popup.details = _("A required file is not available");
					} else {
						popup.details = message;
					}

				}


			} catch(const Udjat::Exception &e) {

				debug("Udjat::exception");
				popup.message = e.title();
				popup.details = e.body();
				Logger::String{e.body()}.error(name());

			} catch(const std::logic_error &e) {

				debug("std::logic_error");
				popup.message = _("Configuration or logic error");
				popup.details = e.what();
				Logger::String{e.what()}.error(name());

			} catch(const std::system_error &e) {

				debug("std::system_error");
				Logger::String details{e.what()," (rc=",e.code().value(),")"};
				details.error(name());
				popup.message = _("System error");
				popup.details = details;

			} catch(const std::exception &e) {

				debug("std::exception");
				popup.message = _("Operation failed");
				popup.details = e.what();
				Logger::String{e.what()}.error(name());

			}

			return -1;
		});

		if(rc) {
			failed.present(popup.message.c_str(),popup.details.c_str());
		} else if(success) {
			Logger::String{success.details()}.info(name());
			success.present(popup.message.c_str(),popup.details.c_str());
		} else {
			Logger::String{"Completed, no success dialog"}.info(name());
		}

	}

	bool Action::getProperty(const char *key, std::string &value) const {

		if(!strncasecmp(key,"boot.",5)) {

			const char *ptr = (key+5);
			if(!strncasecmp(ptr,"path.",5)) {

				ptr += 5;

				// Get boot dir attributes.
				Config::Value<string> path{"boot","path","/boot"};

				struct stat st;
				if(stat(path.c_str(),&st)) {
					throw system_error(errno, system_category(), _("Error getting boot path info"));
				}

				if( !(st.st_mode & S_IFDIR) ) {
					throw runtime_error(Logger::Message(_("{} is not a directory"),path.c_str()));
				}

				FILE *fp;
				struct mntent *fs;
				fp = setmntent("/etc/mtab", "r");
				if (fp == NULL) {
					throw system_error(errno, system_category(), _("Error opening /etc/mtab"));
				}

				struct mntent mnt;
				char buf[PATH_MAX*3];
				std::string mountpoint;

				while ((fs = getmntent_r(fp,&mnt,buf,sizeof(buf))) != NULL) {
					debug(fs->mnt_dir);
					struct stat stm;
					if(stat(fs->mnt_dir,&stm) == 0 && stm.st_dev == st.st_dev) {
						mountpoint = fs->mnt_dir;
						break;
					}

				}
				endmntent(fp);

				if(mountpoint.empty()) {
					throw runtime_error(Logger::Message{_("Cant find mountpoint for '{}'"),path.c_str()});
				}

				Logger::String{"Got mountpoint '",mountpoint.c_str(),"' for path '",path.c_str(),"'"}.write(Logger::Debug,name());

				if(!strcasecmp(ptr,"mount")) {
					value = mountpoint;
					return true;
				}

				if(!strcasecmp(ptr,"relative")) {

					debug(path.c_str());
					if(realpath(path.c_str(),buf) == NULL) {
						throw system_error(errno, system_category(), _("Error readlink boot link"));
					}

					debug(buf);

					if(strncmp(mountpoint.c_str(),buf,mountpoint.size())) {
						throw runtime_error(Logger::Message{"Path '{}' is not inside mount point '{}'",buf,mountpoint.c_str()});
					}

#ifdef DEBUG
					value = "/tmp/";
					value += (buf+mountpoint.size());
#else
					value = (buf+mountpoint.size());
#endif // DEBUG
					return true;

				}

			}

			throw runtime_error(Logger::Message{_("Required attribute '{}' not found"),key});
			return false;
		}

		if(!strcasecmp(key,"install-version")) {
			value = PACKAGE_VERSION;
			return true;
		}

		if(!(strcasecmp(key,"boot-label"))) {
			value = Config::Value<string>{"defaults","boot-label",_("Reinstall workstation")};
			return true;
		}

		if(!(strcasecmp(key,"boot-label-vnc"))) {
			value = Config::Value<string>{"defaults","boot-label-vnc",_("Remote controlled installation")};
			return true;
		}

		if(!strcasecmp(key,"install-kloading")) {
			value = _("Loading kernel...");
			return true;
		}

		if(!strcasecmp(key,"install-iloading")) {
			value = _("Loading installer...");
			return true;
		}

		if(Udjat::NamedObject::getProperty(key,value)) {
			return true;
		}

		Logger::String{"Unable to expand property '",key,"'"}.warning(name());

		return false;
	}


 }

*/