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

	class Action : public Reinstall::Action {
	private:
		Udjat::Dialog output;
		iso9660::Image::Settings imgdef;
		vector<Reinstall::DataSource> sources;

	public:

		Action(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Reinstall::Action{parent,node}, output{"select-device",node}, imgdef{node} {

			if(!(args.icon_name && *args.icon_name)) {
				args.icon_name = "drive-harddisk-usb-symbolic";
			}

			// Load sources.
			Reinstall::DataSource::load(node,sources);

		}

		~Action() {
		}

		int activate(Udjat::Dialog::Progress &progress) override {

			progress = _("Getting required files");
			list<DataSource> files;

			for(auto &source : sources) {
				source.for_each(progress,[&files](const DataSource &value){
					files.push_back(value);
					return false;
				});
			}

			Logger::String{files.size()," files to download"}.trace(name());

			// Build image
			iso9660::Image image{imgdef};

			for(auto &file : files) {
				image.append(file);
			}

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
