/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/singleton.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/script.h>
 #include <stdexcept>
 #include <unordered_map>
 #include <vector>

 #include <libreinstall/action.h>
 #include <libreinstall/source.h>
 #include <libreinstall/repository.h>
 #include <libreinstall/driverupdatedisk.h>
 #include <udjat/ui/dialogs/progress.h>

 #include <libreinstall/writer.h>
 #include <libreinstall/writers/file.h>
 #include <libreinstall/writers/usb.h>

 #include <libreinstall/kernelparameter.h>

 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Action::Action(const XML::Node &node) : NamedObject{node}, Udjat::Menu::Item{node}, dialogs{node}, options{node}, output{node} {

		// Load repositories.
		search(node,"repository",[this](const pugi::xml_node &node){
			repositories.emplace_back(node);
			return false;
		});

		// Load kernel parameters.
		search(node,"kernel-parameter",[this](const pugi::xml_node &node){

#if UDJAT_CHECK_VERSION(1,2,0)
			const char * reponame = Udjat::XML::StringFactory(node,"repository");

			if(!(reponame && *reponame)) {

				// It's a normal parameter
				kparms.push_back(Kernel::Parameter::factory(node));

			} else {

				// It's a repository parameter.
				const_cast<Repository &>(repository(reponame)).set_kernel_parameter(node);

			}
#else
			auto reponame = Udjat::XML::StringFactory(node,"repository");

			if(reponame.empty()) {

				// It's a normal parameter
				kparms.push_back(Kernel::Parameter::factory(node));

			} else {

				// It's a repository parameter.
				const_cast<Repository &>(repository(reponame.c_str())).set_kernel_parameter(node);

			}
#endif // UDJAT_CHECK_VERSION

			return false;
		});

		// Load DUDs
		search(node,"driver-update-disk",[this](const pugi::xml_node &node){
			auto dud = DriverUpdateDisk::factory(node);
			sources.push_back(dud);
			kparms.push_back(dud);
			return false;
		});

		// Load templates.
		search(node,"template",[this](const pugi::xml_node &node){
			templates.emplace_back(node);
			return false;
		});

		// Load scripts.
		search(node,"script",[this](const pugi::xml_node &node){
			scripts.emplace_back(node);
			return false;
		});

		// debug("Repositories.size()=",repositories.size()," sources.size()=",sources.size()," templates.size()=",templates.size());
	}

	Action::OutPut::OutPut(const Udjat::XML::Node &node)
#if UDJAT_CHECK_VERSION(1,2,0)
		: filename{XML::QuarkFactory(node,"output-file-name")},
		  length{String{XML::StringFactory(node,"length")}.as_ull()} {
#else
		: filename{XML::QuarkFactory(node,"output-file-name").c_str()},
		  length{XML::StringFactory(node,"length").as_ull()} {
#endif // UDJAT_CHECK_VERSION

	}

	Action::BootOptions::BootOptions(const Udjat::XML::Node &node)
#if UDJAT_CHECK_VERSION(1,2,0)
		: label{XML::QuarkFactory(node,"boot-label")}, theme{XML::QuarkFactory(node,"boot-theme")} {
#else
		: label{XML::QuarkFactory(node,"boot-label").c_str()}, theme{XML::QuarkFactory(node,"boot-theme").c_str()} {
#endif // UDJAT_CHECK_VERSION

		if(!(label && *label) && node.parent()) {
			label = Logger::Message{
						Config::Value<string>("boot","label-template",_("Install {}")).c_str(),
						node.parent().attribute("name").as_string()
					}.as_quark();
		}

	}

	Action::~Action() {
	}

	const Reinstall::Repository & Action::repository(const char *name) const {

		for(const Repository &repository : repositories) {
			if(repository == name) {
				return repository;
			}
		}

		throw runtime_error(Logger::String("Cant find repository '",name,"'"));

	}

	void Action::prepare(Dialog::Progress &progress, Source::Files &files) const {

		progress.message(_("Preparing"));
		{
			for(const Action::Script &script : scripts) {
				if(script == Script::Pre) {
					progress.url(script.c_str());
					if(script.run(*this)) {
						throw runtime_error(_("Preparation script has failed"));
					}
				}
			}
		}

		progress.message(_("Getting file list"));
		{

			/// @brief Resolved repositories.
			std::unordered_map<std::string, Udjat::URL> urls;

			for(auto source : sources) {

				URL remote{source->remote()};

				const char *reponame = source->repository();
				debug("reponame='",reponame,"'");
				if(reponame && *reponame && (remote[0] == '/' || remote[0] == '.')) {

					// Using repository, resolve real URLs
					const Repository &repo = this->repository(reponame);

					auto it = urls.find(string{reponame});
					if(it == urls.end()) {

						// Not in cache, search for repository real URL
						auto url = repo.url();

						auto result = urls.insert({string{reponame},url});
						if(!result.second) {
							throw runtime_error(Logger::String{"Unable to insert repository '",reponame,"' on URL cache"});
						}

						it = result.first;

					}

					// Resolve local path.
					URL local{source->filename()};

					if(local.empty()) {
						local = repo.path(remote.ComponentsFactory().path.c_str());
					} else {
						local = repo.path(local.ComponentsFactory().path.c_str());
					}

					// Get file list
					Udjat::URL u{it->second};
					u += remote.c_str();

					debug("Local=",local.c_str());
					// debug("Repository URL=",it->second.c_str());
					debug("Remote=",u.c_str());

					source->prepare(
						local,				// URL on local file system.
						u,					// URL for remote repository.
						files				// File list.
					);

				} else {

					// Not using repository, just get file list.
					source->prepare(files);

				}
			}
		}

		if(!templates.empty()) {

			progress.message(_("Applying templates"));
			info() << "Applying " << templates.size() << " template(s)" << endl;

			for(const Reinstall::Template &tmpl : templates) {
				files.apply(*this,tmpl);
			}

		} else {

			info() << "No templates" << endl;

		}

	}

	std::string Action::getProperty(const char *key) const {
		std::string value;
		if(getProperty(key,value)) {
			return value;
		}
		throw runtime_error(Logger::Message{_("Unable to get value for '{}'"),key});
	}

	bool Action::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"boot-label")) {

			if(options.label && *options.label) {
				value = options.label;
			} else {
				value = Udjat::Config::Value<string>("boot","label",_("Install system"));
			}

			return true;
		}

		if(!strcasecmp(key,"boot-theme")) {

			if(options.theme && *options.theme) {
				value = options.theme;
			} else {
				value = Udjat::Config::Value<string>("boot","theme","SLE");
			}

			return true;
		}

		if(strcasecmp(key,"label") == 0 || strcasecmp(key,"install-label") == 0 ) {

			if(options.label && *options.label) {
				value = options.label;
			} else {
				value = Udjat::Config::Value<string>("boot","label",_("Reinstall this workstation"));
			}
			return true;

		}

		if(strcasecmp(key,"install-kloading") == 0) {

			value = _("Loading installation kernel ...");
			return true;

		}

		if(strcasecmp(key,"install-iloading") == 0) {

			value = _("Loading system installer ...");
			return true;

		}

		if(strcasecmp(key,"install-version") == 0) {

			// FIX-ME: Use kernel version.
			value = PACKAGE_VERSION;
			return true;

		}

		if(!strcasecmp(key,"kernel-parameters")) {

			for(auto kparm : kparms) {

				if(!value.empty()) {
					value += " ";
				}

				value += kparm->name();

				auto val = kparm->value();
				if(!val.empty()) {
					value += "=";
					value += kparm->value();
				}

			}

			for(const Repository &repository : repositories) {

				const Kernel::Parameter &kparm{repository.kernel_parameter()};

				if(!kparm) {
					continue;
				}

				if(!value.empty()) {
					value += " ";
				}

				value += kparm.name();

				auto val = kparm.value();
				if(!val.empty()) {
					value += "=";
					value += kparm.value();
				}

			}

			Logger::String{"Kernel-parameters: '",value,"'"}.write(Logger::Debug,name());

			return true;
		}

		if(!strcasecmp(key,"template-dir")) {
#ifdef DEBUG
			value = "./templates";
#else
			value = Udjat::Application::DataDir{"templates"};
#endif // DEBUG
			return true;
		}

		if(!Udjat::NamedObject::getProperty(key,value)) {
			Logger::Message message{_("Cant get property '{}'"),key};
			message.error(name());
			throw runtime_error{message};
		}

		return true;

	}

	void Action::build(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, Source::Files &files) const {

		progress.pulse();
		progress.message(_("Building image"));
		builder->pre();

		progress.message(_("Getting required files"));
		{
			size_t current = 0;
			size_t total = files.size();
			files.for_each([builder,&progress,&current,total](std::shared_ptr<Source::File> file){
				progress.count(++current,total);
				builder->push_back(file);
			});

		}

		if(!templates.empty()) {
			progress.message(_("Applying templates"));
			info() << "Applying " << templates.size() << " template(s)" << endl;
			builder->push_back(*this,templates);
		} else {
			info() << "No templates" << endl;
		}

		progress.message(_("Building image"));
		builder->post();

		for(const Action::Script &script : scripts) {
			if(script == Script::Post) {
				progress.url(script.c_str());
				if(script.run(*this)) {
					throw runtime_error(_("Post script has failed"));
				}
			}
		}

		progress.message(_("Build process complete"));
	}

	void Action::write(Dialog::Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::shared_ptr<Reinstall::Writer> writer) const {

		progress.pulse();
		progress.message(_("Writing image"));
		builder->write(writer);

	}

	std::shared_ptr<Reinstall::Builder> Action::BuilderFactory() const {
		throw runtime_error(_("The selected action is unable to build an image"));
	}

	std::shared_ptr<Reinstall::Writer> Action::WriterFactory() const {

		if(output.filename && *output.filename) {
			return make_shared<Reinstall::FileWriter>(output.filename);
		}

		debug("Calling default writer");
		return Reinstall::Writer::factory(dialogs.title);
	}

	bool Action::confirm() const {
		return dialogs.confirmation.confirm();
	}

	void Action::activate() {

		class Files : public Reinstall::Source::Files {
		private:
			std::set<std::shared_ptr<Reinstall::Source::File>> files;

		public:
			void insert(std::shared_ptr<Reinstall::Source::File> file) override {
				files.insert(file);
			}

			size_t size() const override {
				return files.size();
			}

			void for_each(const std::function<void(std::shared_ptr<Reinstall::Source::File>)> &worker) override {
				for(auto file : files) {
					worker(file);
				}
			}

			void remove_if(const std::function<bool(const Reinstall::Source::File &)> &worker) override {

				for(auto it = files.begin(); it != files.end();) {
					if(worker(*(*it))) {
						it = files.erase(it);
					} else {
						it++;
					}
				}

			}

			void apply(const Udjat::Abstract::Object &object,const Template &tmpl) override {

				vector<string> updates;

				for(auto it = files.begin(); it != files.end();) {

					std::shared_ptr<Reinstall::Source::File> file = *it;

					if(tmpl.test(file->c_str())) {

						Logger::String{"Replacing '",file->c_str(),"'"}.trace(tmpl.name());
						it = files.erase(it);
						updates.push_back(file->c_str());

					} else {

						it++;

					}

				}

				if(updates.size()) {

					// Have updates, get template.
					String text{tmpl.apply(object)};
					for(string &path : updates) {
						insert(Source::File::Factory(path.c_str(),text.c_str()));
					}

				}

			}

		};

		if(!this->confirm()) {
			Logger::String{"Cancel by user choice"}.info(name());
			return;
		}

		Dialog::Controller &dcntrl{Dialog::Controller::instance()};

		try {

			Files files;

			// Step 1, get files, prepare for build.
			Logger::String{"Preparing"}.info(name());
			dcntrl.run(dialogs.progress,[this,&files](Dialog::Progress &dialog) {
				dialog.title(dialogs.title);
				dialog.message(_("Preparing"));
				prepare(dialog,files);
				return 0;
			});

			// Step 2, build image.
			Logger::String{"Building"}.info(name());
			std::shared_ptr<Reinstall::Builder> builder{BuilderFactory()};
			dcntrl.run(dialogs.progress,[this,builder,&files](Dialog::Progress &dialog) {
				dialog.title(dialogs.title);
				dialog.message(_("Building image"));
				build(dialog,builder,files);
				return 0;
			});

			// Step 3, get writer.
			debug("Getting writer");
			std::shared_ptr<Reinstall::Writer> writer{WriterFactory()};
			debug("Got writer");

			// Step 4, write image.
			Logger::String{"Writing"}.info(name());
			dcntrl.run(dialogs.progress,[this,builder,writer](Dialog::Progress &dialog) {
				dialog.title(dialogs.title);
				dialog.message(_("Writing"));
				write(dialog,builder,writer);
				return 0;
			});

			dialogs.success.run();

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(name());
			dialogs.failed.run(e.what());

		} catch(...) {

			Logger::String{"Unexpected error"}.error(name());
			dialogs.failed.run(_("Unexpected error"));

		}

	}

 }

