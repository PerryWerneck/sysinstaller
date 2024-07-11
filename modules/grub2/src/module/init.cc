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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/action.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/writer.h>
 #include <udjat/ui/dialog.h>
 #include <vector>

 using namespace Udjat;
 using namespace std;
 using namespace Reinstall;

 /// @brief Register udjat module.
 UDJAT_API Udjat::Module * udjat_module_init() {

	static const Udjat::ModuleInfo moduleinfo{
          "Reinstallation without disk image."
	};

	class Action : public Reinstall::Action {
	private:

		class Kernel : public Reinstall::FileSource {
		public:
			Kernel(const Udjat::XML::Node &node) : FileSource{node,"kernel"} {
				url.local = "file://${boot-mountpoint}${boot-path}/${kernel-name}";
				url.path = "${boot-path}/${kernel-name}";

				Logger::String{"Source from ",url.remote}.trace(name());

				if(!(message && *message)) {
					message = _("Getting installation kernel");
				}
			}
		};

		class Init : public Reinstall::FileSource {
		public:
			Init(const Udjat::XML::Node &node) : FileSource{node,"init"} {
				url.local = "file://${boot-mountpoint}${boot-path}${init-name}";
				url.path = "${boot-path}/${init-name}";

				Logger::String{"Source from ",url.remote}.trace(name());

				if(!(message && *message)) {
					message = _("Getting init system");
				}
			}
		};

		std::vector<std::shared_ptr<Reinstall::DataSource>> sources;

	public:
		Action(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Reinstall::Action{parent,node} {

			sources.push_back(make_shared<Kernel>(node));
			sources.push_back(make_shared<Init>(node));

		}

		int activate(Udjat::Dialog::Progress &progress) override {

			progress = _("Getting required files");
			for(const auto &source : sources) {
				source->save(progress);
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
