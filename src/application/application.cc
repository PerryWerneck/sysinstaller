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
  * @brief The reinstall application.
  */

 #include <config.h>
 #include <udjat/tools/application.h>
 #include <reinstall/application.h>
 #include <udjat/tools/factory.h>
 #include <udjat/module/info.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/url/handler/http.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <string>
 #include <reinstall/action.h>
 #include <udjat/tools/string.h>
 #include <udjat/ui/status.h>

 #include <mntent.h>
 #include <limits.h>

 #include <reinstall/modules/grub2.h>
 #include <reinstall/modules/isowriter.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Application *Application::instance = nullptr;
	static const Udjat::ModuleInfo moduleinfo{"Top menu option"};
		
	Application::Application() : Factory{"group",moduleinfo} {

		// Load default http handler.
		static HTTP::Handler::Factory http_handler{"default"};

		if(instance) {
			throw std::runtime_error{"Application already created"};
		}

		instance = this;
		Logger::String{"Creating application"}.info();

		// Setup global expansion.
		String::push_back([](const char *key, std::string &value, bool, bool) -> bool{

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

					Logger::String{"Got mountpoint '",mountpoint.c_str(),"' for path '",path.c_str(),"'"}.write(Logger::Debug);

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

			return false;
			
		});

		// Load modules.

#ifndef _WIN32
		if(Config::Value<bool>{"modules","grub2",true}) {
			Reinstall::Grub2::Module::Factory("grub");
		}
#endif

		if(Config::Value<bool>{"modules","isowriter",true}) {
			Reinstall::IsoWriter::Module::Factory();
		}

	}

	Application::~Application() {
		Module::unload();
		instance = nullptr;
	}

	Application & Application::getInstance() {
		if(!instance) {
			throw std::runtime_error{"Application not created"};
		}
		return *instance;
	}

	void Application::select(std::shared_ptr<Action> a) {
		Logger::String("Action '",a->name(),"' was selected").trace();
		action = a;
	}

	void Application::load_options() {

		Logger::String{"Loading options"}.info();

	#ifdef DEBUG
		XML::parse("./xml.d");
	#else
		XML::parse();
	#endif

	}

	void Application::push_back(const Udjat::XML::Node &node, std::shared_ptr<Reinstall::Action> child) {

		debug("Adding action '",child->name(),"'");

		string parent{node.parent().attribute("name").as_string("default")};

		auto result = groups.find(parent);
		if(result == groups.end()) {
			throw runtime_error{Logger::String{"Group '",parent.c_str(),"' not found"}};
		}

		result->second->push_back(node,child);

	}

	bool Application::parse(const Udjat::XML::Node &node) {

		string name{node.attribute("name").as_string("default")};
		std::shared_ptr<Group> group;

		auto result = groups.find(name);
		if(result == groups.end()) {
			Logger::String{"Building group '",name.c_str(),"'"}.trace();
			group = group_factory(node);
			groups.insert({name,group});
		} else {
			Logger::String{"Updating group '",name.c_str(),"'"}.trace();
			group = result->second;
		}

		group->setup(node);

		return true;
	}

	void Application::activate() noexcept {

		if(!action.get()) {
			Logger::String{"No action selected"}.error();
			return;
		}

		Logger::String{"Activating action"}.info(action->name());

		auto &status = Udjat::Dialog::Status::getInstance();

		status.title(action->title());
		status.sub_title(_("Initializing..."));
		status.icon(action->icon());

		try {
			action->activate();
			action->success->present();
		} catch(const std::exception &e) {
			action->failed->present(e);
		} catch(...) {
			action->failed->present(std::runtime_error{_("Unknown error")});
		}

	}

	
 }
 