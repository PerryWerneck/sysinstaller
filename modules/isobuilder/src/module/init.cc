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

 #ifdef HAVE_ISOFS
	#include <iso9660.h>
 #endif // HAVE_ISOFS

 #ifdef HAVE_FATFS
	#include <fatfs.h>
 #endif // HAVE_FATFS

 #include <unistd.h>

 using namespace Udjat;
 using namespace std;
 using namespace Reinstall;

 /// @brief Register udjat module.
 UDJAT_API Udjat::Module * udjat_module_init() {

	static const Udjat::ModuleInfo moduleinfo{
          "Build customized installation image."
	};

	/// @brief Base class for actions.
	class UDJAT_PRIVATE Action : public Reinstall::Action, protected Reinstall::Builder {
	protected:
		virtual void build(Udjat::Dialog::Progress &progress, list<std::shared_ptr<DataSource>> &files) = 0;

	public:
		Action(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Reinstall::Action{parent,node}, Reinstall::Builder{*this,node} {

			if(!(args.icon_name && *args.icon_name)) {
				args.icon_name = "drive-harddisk-usb-symbolic";
			}

		}

		inline const char *name() const noexcept {
			return Reinstall::Action::name();
		}

		bool getProperty(const char *key, std::string &value) const override {

			if(Builder::getProperty(key,value)) {
				return true;
			}

			return Reinstall::Action::getProperty(key,value);
		}

		int activate(Udjat::Dialog::Progress &progress) override {

			list<std::shared_ptr<DataSource>> files;
			prepare(progress,files);

			build(progress,files);

			return 0;
		}

	};

#ifdef HAVE_ISOFS
	/// @brief ISO9660 builder.
	class UDJAT_PRIVATE Iso9660Builder : public Action {
	private:
		iso9660::Image::Settings imgdef;

	protected:
		void build(Udjat::Dialog::Progress &progress, list<std::shared_ptr<DataSource>> &files) override {

			// Build image ...
			Logger::String{"Building ISO-9660 Image"}.info(name());

			iso9660::Image image{output,*this,imgdef};

			image.pre(*this);
			image.append(progress,files);
			image.post(*this);

			// ... and write it to device.
			debug("Complete, writing...");
			image.write(progress);
		}

	public:

		Iso9660Builder(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Action{parent,node}, imgdef{node} {
		}


	};
#endif // HAVE_ISOFS

#ifdef HAVE_FATFS
	/// @brief Fat builder.
	class UDJAT_PRIVATE FatBuilder : public Action {
	private:
		FatFS::Image::Settings imgdef;

	protected:
		void build(Udjat::Dialog::Progress &progress, list<std::shared_ptr<DataSource>> &files) override {

			// Build image ...
			Logger::String{"Building Fat Image"}.info(name());

			// ... and write it to device.
			debug("Complete, writing...");

			throw runtime_error("FAT image is incomplete");
		}

	public:

		FatBuilder(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Action{parent,node}, imgdef{node} {
		}


	};
#endif // HAVE_FATFS

	class Module : public Udjat::Module, public Udjat::Factory {
	private:

		/// @brief Proxy for legacy config files.
		class Proxy : public Udjat::Factory {
		private:
			Udjat::Factory &target ;

		public:
			Proxy(Udjat::Factory *t) : Udjat::Factory("network-installer",moduleinfo), target{*t} {
			}

			std::shared_ptr<Udjat::Abstract::Object> ObjectFactory(const Udjat::Abstract::Object &parent, const XML::Node &node) override {
				Logger::String{"Parsing obsolete node '",node.name(),"'"}.warning(node.attribute("name").as_string("node"));
				return target.ObjectFactory(parent,node);
			}

		};

		Proxy legacy;

	public:
		Module() : Udjat::Module("isobuilder",moduleinfo), Udjat::Factory("iso-builder",moduleinfo), legacy{this} {
		};

		// Udjat::Factory
		std::shared_ptr<Udjat::Abstract::Object> ObjectFactory(const Udjat::Abstract::Object &parent, const XML::Node &node) override {

			auto attr = XML::AttributeFactory(node,"filesystem");

#ifdef HAVE_ISOFS
			if(strcasecmp(attr.as_string("iso9660"),"iso9660") == 0) {
				return make_shared<Iso9660Builder>(parent,node);
			}
#endif // HAVE_ISOFS

#ifdef HAVE_FATFS
			if(strcasecmp(attr.as_string("fat"),"fat") == 0 || strcasecmp(attr.as_string("fat32"),"fat32") == 0) {
				return make_shared<FatBuilder>(parent,node);
			}
#endif // HAVE_FATFS

			Logger::String{"Unexpected value for attribute filesystem: '",attr.as_string(),"'"}.warning(Factory::name());
			return std::shared_ptr<Udjat::Abstract::Object>();
		}

	};

	return new Module();
 }
