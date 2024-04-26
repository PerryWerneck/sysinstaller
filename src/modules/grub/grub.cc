/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/module.h>
 #include <udjat/factory.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/string.h>

 #include <libreinstall/source.h>
 #include <libreinstall/builder.h>
 #include <libreinstall/repository.h>
 #include <libreinstall/kernelparameter.h>
 #include <libreinstall/action.h>

 #include <memory>

 using namespace Udjat;
 using namespace Reinstall;
 using namespace std;

#ifdef DEBUG
	#define BOOT_PATH "/tmp"
#else
	#define BOOT_PATH "/boot"
#endif // DEBUG

 Udjat::Module * udjat_module_init() {


 	/// @brief The 'image' builder.
 	class Builder : public Reinstall::Builder {
	private:
		const Reinstall::Action &action;

	public:
		constexpr Builder(const Reinstall::Action &a) : Reinstall::Builder{"grub-setup"}, action{a} {
		}

		void pre() override {
		}

		void post() override {
		}

		void push_back(std::shared_ptr<Reinstall::Source::File> file) override {

			String path{file->c_str()};

			path.expand(action);

			// Allways remove to prevent caching.
			if(::remove(path.c_str()) && errno != ENOENT) {
				throw system_error(errno,system_category(),file->c_str());
			}

			Logger::String{"Saving '",path.c_str(),"'"}.info("grub");

			File::Handler output{path.c_str(),true};
			Dialog::Progress &dialog{Dialog::Progress::instance()};
			file->save([&output,&dialog](unsigned long long offset, unsigned long long total, const void *buf, size_t length){
				output.write(offset, buf, length);
				dialog.progress(offset,total);
			});

		}

		void push_back(const Udjat::Abstract::Object &object, const std::vector<Reinstall::Template> &templates) override {

			Dialog::Progress &dialog{Dialog::Progress::instance()};
			dialog.message(_("Saving control files"));

			for(const Reinstall::Template &tmpl : templates) {
				const char *path = tmpl.path();

				if(path && *path) {
					auto source = tmpl.SourceFactory(object,path);

					String filename{path};
					filename.expand(object);

					Logger::String{"Saving '",filename.c_str(),"'"}.info("grub");

					dialog.url(filename.c_str());

					File::Handler output{filename.c_str(),true};
					source->save([&output,&dialog](unsigned long long offset, unsigned long long total, const void *buf, size_t length){
						output.write(offset, buf, length);
						dialog.progress(offset,total);
					});

					dialog.url();
					dialog.pulse();

				}

			}

		}

		void write(std::shared_ptr<Writer>) override {
		}

 	};

	/// @brief The source for kernel file.
	class Kernel : public Source {
	public:
		Kernel(const Udjat::XML::Node &node) : Source{node,false} {

			if(!(imgpath && *imgpath)) {
				imgpath = "${boot-path}/kernel." PACKAGE_NAME;
			}

		}
	};

	/// @brief The source for init file.
	class InitRD : public Source {
	public:
		InitRD(const Udjat::XML::Node &node) : Source{node,false} {

			if(!(imgpath && *imgpath)) {
				imgpath = "${boot-path}/initrd." PACKAGE_NAME;
			}

		}
	};

	class Action : public Reinstall::Action {
	private:
		std::shared_ptr<Kernel> kernel;
		std::shared_ptr<InitRD> initrd;

	public:

		Action(const Udjat::XML::Node &node) : Reinstall::Action{node} {

			search(node,"kernel",[this](const pugi::xml_node &node){
				this->kernel = make_shared<Kernel>(node);
				sources.push_back(this->kernel);
				return true;
			});

			// Get init source.
			search(node,"init",[this](const pugi::xml_node &node){
				this->initrd = make_shared<InitRD>(node);
				sources.push_back(this->initrd);
				return true;
			});

		}

		std::shared_ptr<Reinstall::Builder> BuilderFactory() const override {
			return make_shared<Builder>(*this);
		}

		bool getProperty(const char *key, std::string &value) const override {

			debug("Searching for '",key,"' in ",name());

			if(strcasecmp(key,"boot-path") == 0 || strcasecmp(key,"grub-path") == 0) {

				// TODO: Detect boot path ( /proc/cmdline? )
#ifdef DEBUG
				value = "/tmp";
#else
				value = "/boot";
#endif // DEBUG
				return true;
			}

			if(strcasecmp(key,"kernel-file") == 0 || strcasecmp(key,"kernel-rpath") == 0) {
				value = Source::rpath(String{kernel->image_path()}.expand(*this).c_str(),name());
				return true;
			}

			if(strcasecmp(key,"initrd-file") == 0 || strcasecmp(key,"initrd-rpath") == 0) {
				value = Source::rpath(String{initrd->image_path()}.expand(*this).c_str(),name());
				return true;
			}

			if(strcasecmp(key,"grub-conf-dir") == 0) {
#ifdef DEBUG
				value = "/tmp";
#else
				value = "/etc/grub.d";
#endif // DEBUG
				return true;
			}

			return Reinstall::Action::getProperty(key,value);
		}


	};

 	static const Udjat::ModuleInfo moduleinfo { "Grub based system installation" };

 	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("local-installer", moduleinfo), Udjat::Factory("local-installer",moduleinfo) {
		}

		bool generic(const XML::Node &node) override {
			new Action(node);
			return false;
		}
 	};

 	return new Module();

 }

 /*
 #include <udjat/factory.h>
 #include <udjat/tools/object.h>
 #include <stdexcept>
 #include <reinstall/userinterface.h>
 #include <reinstall/group.h>
 #include <reinstall/builder.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/source.h>
 #include <reinstall/sources/kernel.h>
 #include <reinstall/sources/initrd.h>
 #include <reinstall/action.h>

 using namespace std;
 using namespace Udjat;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "Grub based system installation" };

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("local-installer", moduleinfo), Udjat::Factory("local-installer",moduleinfo) {
		}

		bool generic(const pugi::xml_node &node) override {

			class Action : public Reinstall::Action {
			private:
				std::shared_ptr<Reinstall::Kernel> kernel;
				std::shared_ptr<Reinstall::InitRD> initrd;

			public:
				Action(const pugi::xml_node &node) : Reinstall::Action(node,"computer") {

					// Get URL for installation kernel.
					if(!scan(node,"kernel",[this](const pugi::xml_node &node) {
						kernel = make_shared<Reinstall::Kernel>(node);
						push_back(kernel);
						return true;
					})) {
						throw runtime_error(_("Missing required entry <kernel> with the URL for installation kernel"));
					}

					// Get URL for installation init.
					if(!scan(node,"init",[this](const pugi::xml_node &node) {
						initrd = make_shared<Reinstall::InitRD>(node);
						push_back(initrd);
						return true;
					})) {
						throw runtime_error(_("Missing required entry <init> with the URL for the linuxrc program"));
					}

					debug("sources.size=",sources.size());

				}

				std::shared_ptr<Reinstall::Builder> BuilderFactory() override {

					class Builder : public Reinstall::Builder {
					public:
						Builder() = default;

						void pre(const Reinstall::Action &action) override {
						}

						bool apply(Reinstall::Source &source) override {

							if(!Reinstall::Builder::apply(source)) {
								return false;
							}

							source.save(source.path);

							return true;

						}

						void build(Reinstall::Action &action) override {
						}

						void post(const Reinstall::Action &action) override {
						}

						std::shared_ptr<Reinstall::Writer> burn(std::shared_ptr<Reinstall::Writer> writer) override {

							return writer;
						}


					};

					return(make_shared<Builder>());

				}

				std::shared_ptr<Reinstall::Writer> WriterFactory() override {
					return make_shared<Reinstall::Writer>(*this);
				}

				std::string getProperty(const char *key) const {
					std::string value;
					if(getProperty(key,value)) {
						return value;
					}
					throw runtime_error(Logger::Message{_("Unable to get value for '{}'"),key});
				}

				bool getProperty(const char *key, std::string &value) const override {

					debug("Searching for '",key,"' in ",name());

					if(strcasecmp(key,"kernel-fspath") == 0 || strcasecmp(key,"kernel-rpath") == 0 ) {
						value = (kernel->rpath()+1);
						return true;
					}

					if(strcasecmp(key,"initrd-fspath") == 0 || strcasecmp(key,"initrd-rpath") == 0 ) {
						value = (initrd->rpath()+1);
						return true;
					}

					if(strcasecmp(key,"boot-dir") == 0 || strcasecmp(key,"grub-path") == 0) {

						// TODO: Detect boot path ( /proc/cmdline? )
#ifdef DEBUG
						value = "/tmp";
#else
						value = "/boot";
#endif // DEBUG
						return true;
					}

					if(strcasecmp(key,"grub-conf-dir") == 0) {
#ifdef DEBUG
						value = "/tmp";
#else
						value = "/etc/grub.d";
#endif // DEBUG
						return true;
					}

					if(strcasecmp(key,"install-enabled") == 0) {
						value = "1";
						return true;
					}

					if(strcasecmp(key,"kernel-path") == 0) {
						// TODO: Detect boot partition
						value = "/boot/kernel-" PACKAGE_NAME;
						debug(key,"=",value);
						return true;
					}

					if(strcasecmp(key,"initrd-path") == 0) {
						// TODO: Detect boot partition
						value = "/boot/initrd-" PACKAGE_NAME;
						debug(key,"=",value);
						return true;
					}

					if(strcasecmp(key,"kernel-filename") == 0) {
						value = "kernel." PACKAGE_NAME;
						return true;
					}

					if(strcasecmp(key,"initrd-filename") == 0) {
						value = "initrd." PACKAGE_NAME;
						return true;
					}

					if(strcasecmp(key,"kernel-file") == 0) {
						value = getProperty("grub-path") + "/" + getProperty("kernel-filename");
						return true;
					}

					if(strcasecmp(key,"initrd-file") == 0) {
						value = getProperty("grub-path") + "/" + getProperty("initrd-filename");
						return true;
					}

					if(Reinstall::Action::getProperty(key,value)) {
						return true;
					}

					throw runtime_error(Logger::Message{_("Unable to get value for '{}'"),key});

				}

			};

			Reinstall::push_back(node,make_shared<Action>(node));

			return true;
		}

	};

	return new Module();

 }
 */

