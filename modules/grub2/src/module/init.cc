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
 #include <udjat/module.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/factory.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/action.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/writer.h>
 #include <reinstall/tools/template.h>
 #include <udjat/ui/dialog.h>
 #include <vector>
 #include <reinstall/tools/kernelparameter.h>

 using namespace Udjat;
 using namespace std;
 using namespace Reinstall;

 static const char * PathFactory(const Udjat::Abstract::Object &object, const Udjat::XML::Node &node, const char *name, const char *text) {

	String str{text};

	str.expand('$',[&](const char *key, std::string &value){

		if(!strcasecmp(key,"name")) {
			value = name;
			return true;
		}

		if(!strcasecmp(key,"filename")) {
			value = Config::Value<string>("app-defaults",(string{name}+"-name").c_str(),"${name}.reinstall");
			return true;
		}

		return false;
	});

	str.expand(node);
	str.expand(object);

	return str.as_quark();
 }

 /// @brief Register udjat module.
 UDJAT_API Udjat::Module * udjat_module_init() {

	static const Udjat::ModuleInfo moduleinfo{
          "Reinstallation without disk image."
	};

	class Action : public Reinstall::Action {
	private:

		class Kernel : public Reinstall::FileSource {
		public:
			Kernel(const Udjat::Abstract::Object &object, const Udjat::XML::Node &node) : FileSource{node,"kernel"} {
				url.local = ::PathFactory(object,node,"kernel","file://${boot.path.mount}${boot.path.relative}/${filename}");
				url.path = ::PathFactory(object,node,"kernel","${boot.path.relative}/${filename}");

				if(!(message && *message)) {
					message = _("Getting installation kernel");
				}
			}
		};

		class Init : public Reinstall::FileSource {
		public:
			Init(const Udjat::Abstract::Object &object, const Udjat::XML::Node &node) : FileSource{node,"init"} {
				url.local = ::PathFactory(object,node,"initrd","file://${boot.path.mount}${boot.path.relative}/${filename}");
				url.path = ::PathFactory(object,node,"initrd","${boot.path.relative}/${filename}");

				if(!(message && *message)) {
					message = _("Getting init system");
				}
			}
		};

		std::vector<std::shared_ptr<Reinstall::DataSource>> sources;
		std::vector<std::shared_ptr<Reinstall::KernelParameter>> kparms;
		std::vector<std::shared_ptr<Reinstall::Template>> templates;

		const char *boot_label = _("Reinstall workstation");

	public:
		Action(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Reinstall::Action{parent,node} {

			static const char *labels[] = {
				"grub-label",
				"boot-label",
				"system-name",
				"label",
				"title"
			};

			for(const char *label : labels) {
				const char *ptr = XML::QuarkFactory(node,label);
				if(*ptr) {
					boot_label = ptr;
					break;
				}
			}

			sources.push_back(make_shared<Kernel>(*this,node));
			sources.push_back(make_shared<Init>(*this,node));

			// Load kernel parameters.
			Reinstall::KernelParameter::load(node,kparms);

			// Load templates
			Reinstall::Template::load(*this,node,templates);

		}

		bool getProperty(const char *key, std::string &value) const override {

/*
17/07/2024 00:47:11 tw-local       Unable to expand property 'kernel-file'
17/07/2024 00:47:11 tw-local       Unable to expand property 'initrd-file'
17/07/2024 00:47:11 tw-local       Unable to expand property 'install-version'
*/

			if(!(strcasecmp(key,"boot-label") && strcasecmp(key,"install-label"))) {
				if(boot_label && *boot_label) {
					value = boot_label;
				} else {
					value = _("Reinstall this workstation");
				}
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

			if(!strcasecmp(key,"kernel-parameters")) {
				value = KernelParameter::join(*this,kparms);
				debug("Kernel parameters set to '",value.c_str(),"'");
				return true;
			}

			if(!strcasecmp(key,"grub-conf-dir")) {
				value = Config::Value<string>("grub","conf-dir","/etc/grub.d");
#ifdef DEBUG
				debug("Grub config was set to '",value.c_str(),"'");
				value = "/tmp/" + value;
#endif // DEBUG
				return true;
			}

			return Reinstall::Action::getProperty(key,value);
		}

		int activate(Udjat::Dialog::Progress &progress) override {

			progress = _("Getting required files");
			for(const auto &source : sources) {
				source->save(*this,progress);
			}

			progress = _("Applying templates");
			for(const auto &tmplt : templates) {
				tmplt->save(*this,progress);
			}

			return -1;
		}

	};

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("grub2",moduleinfo), Udjat::Factory("local-installer",moduleinfo) {
		};

		// Udjat::Factory
		std::shared_ptr<Abstract::Object> ObjectFactory(const Abstract::Object &parent, const XML::Node &node) override {
			return make_shared<Action>(parent,node);
		}

	};

	return new Module();
 }
