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
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/module.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/factory.h>

 #include <udjat/ui/menu.h>

 #include <libreinstall/action.h>
 #include <libreinstall/builder.h>
 #include <libreinstall/builders/iso9660.h>
 #include <libreinstall/builders/fat.h>

 #include <stdexcept>

 using namespace std;
 using namespace Udjat;
 using namespace Reinstall;

 Udjat::Module * udjat_module_init() {

 	static const Udjat::ModuleInfo moduleinfo { "Disk image builder" };

 	class iso9660Action : public Reinstall::Action, private iso9660::Settings {
	public:
		iso9660Action(const XML::Node &node) : Reinstall::Action{node}, iso9660::Settings{node} {

			search(node,"source",[this](const pugi::xml_node &node){
				sources.push_back(Source::factory(node));
				return false;
			});

		}

		std::shared_ptr<Reinstall::Builder> BuilderFactory() const override {
			return iso9660::BuilderFactory(*this);
		}

 	};

  	class FatAction : public Reinstall::Action, private iso9660::Settings {
	private:
		unsigned long long length;

	public:
#if UDJAT_CHECK_VERSION(1,2,0)
		FatAction(const XML::Node &node) : Reinstall::Action{node}, length{String{node,"length"}.as_ull()} {

			search(node,"source",[this](const pugi::xml_node &node){
				sources.push_back(Source::factory(node));
				return false;
			});

		}
#else
		FatAction(const XML::Node &node) : Reinstall::Action{node}, length{XML::StringFactory(node,"length").as_ull()} {

			search(node,"source",[this](const pugi::xml_node &node){
				sources.push_back(Source::factory(node));
				return false;
			});

		}
#endif

		std::shared_ptr<Reinstall::Builder> BuilderFactory() const override {
			return Fat::BuilderFactory(length);
		}

 	};

	class Module : public Udjat::Module, public Udjat::Factory {
	public:
		Module() : Udjat::Module("iso-builder", moduleinfo), Udjat::Factory("iso-builder",moduleinfo) {
		}

		bool generic(const XML::Node &node) override {

#if UDJAT_CHECK_VERSION(1,2,0)
			switch(String{node,"filesystem","iso9660"}.select("fat32","iso9660",nullptr)) {
#else
			switch(XML::StringFactory(node,"filesystem","value","iso9660").select("fat32","iso9660",nullptr)) {
#endif // UDJAT_CHECK_VERSION
			case 0:	// FAT32
				new FatAction(node);
				break;

			case 1: // iso9660
				new iso9660Action(node);
				break;

			default:
				return false;
			}

			return true;
		}

	};

	return new Module();

 }

