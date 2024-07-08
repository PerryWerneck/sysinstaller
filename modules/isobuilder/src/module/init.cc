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
 #include <udjat/tools/factory.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/action.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/writer.h>
 #include <reinstall/tools/template.h>
 #include <reinstall/tools/builder.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/progress.h>
 #include <vector>
 #include <list>

 #include <iso9660.h>

 using namespace Udjat;
 using namespace std;
 using namespace Reinstall;

 /// @brief Register udjat module.
 UDJAT_API Udjat::Module * udjat_module_init() {

	static const Udjat::ModuleInfo moduleinfo{
          "Create customized installation image."
	};

	class Action : public Reinstall::Action, private Reinstall::Builder {
	private:
		iso9660::Image::Settings imgdef;

	public:

		Action(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Reinstall::Action{parent,node}, Reinstall::Builder{*this,node}, imgdef{node} {

			if(!(args.icon_name && *args.icon_name)) {
				args.icon_name = "drive-harddisk-usb-symbolic";
			}

		}

		~Action() {
		}

		bool getProperty(const char *key, std::string &value) const override {

			if(Builder::getProperty(key,value)) {
				return true;
			}

			return Reinstall::Action::getProperty(key,value);
		}

		int activate(Udjat::Dialog::Progress &progress) override {

			debug("Action activated");

			list<std::shared_ptr<DataSource>> files;
			prepare(progress,files);

			// Build image ...
			iso9660::Image image{output,*this,imgdef};

			image.pre(*this);
			image.append(progress,files);
			image.post(*this);

			// ... and write it to device.
			image.write(progress);

			return 0;
		}

	};

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("isobuilder",moduleinfo), Udjat::Factory("iso-builder",moduleinfo) {
		};

		// Udjat::Factory
		std::shared_ptr<Udjat::Abstract::Object> ObjectFactory(const Udjat::Abstract::Object &parent, const XML::Node &node) override {
			return make_shared<Action>(parent,node);
		}

	};

	return new Module();
 }
