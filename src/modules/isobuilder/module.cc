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
 #include <udjat/tools/configuration.h>
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
 #include <reinstall/modules/isobuilder.h>
 #include <list>
 #include <reinstall/modules/iso9660.h>
 #include <reinstall/modules/fatfs.h>

 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	static const Udjat::ModuleInfo moduleinfo{
          "Build customized installation image."
	};

	/// @brief Base class for actions.
	class UDJAT_PRIVATE IsoBuilder::Module::Action : public Reinstall::Action, protected Reinstall::Builder {
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

	/// @brief ISO9660 builder.
	class UDJAT_PRIVATE Iso9660Builder : public IsoBuilder::Module::Action {
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

	/// @brief Fat builder.
	class UDJAT_PRIVATE FatBuilder : public IsoBuilder::Module::Action {
	private:
		FatFS::Image::Settings imgdef;

	protected:
		void build(Udjat::Dialog::Progress &progress, list<std::shared_ptr<DataSource>> &files) override {

			// Build image ...
			Logger::String{"Building Fat Image"}.info(name());
			FatFS::Image image{output,*this,imgdef};

			image.pre(*this);
			image.append(progress,files);
			image.post(*this);

			// ... and write it to device.
			debug("Complete, writing...");
			image.write(progress);

		}

	public:

		FatBuilder(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
			: Action{parent,node}, imgdef{node} {
		}


	};

	Reinstall::IsoBuilder::Module::Module(const char *name) : Udjat::Module(name,moduleinfo), Udjat::Factory("iso-builder",moduleinfo) {
	}

	Reinstall::IsoBuilder::Module::~Module() {
	}

	
	std::shared_ptr<Udjat::Abstract::Object> Reinstall::IsoBuilder::Module::ObjectFactory(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node) {
		try {

			auto attr = XML::AttributeFactory(node,"filesystem");

			if(strcasecmp(attr.as_string("iso9660"),"iso9660") == 0) {
				return make_shared<Iso9660Builder>(parent,node);
			}

			if(strcasecmp(attr.as_string("fat"),"fat") == 0 || strcasecmp(attr.as_string("fat32"),"fat32") == 0) {
				return make_shared<FatBuilder>(parent,node);
			}

			Logger::String{"Unexpected value for attribute filesystem: '",attr.as_string(),"'"}.error(Factory::name());

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(Factory::name());

		}

		return std::shared_ptr<Udjat::Abstract::Object>();

	}

	Udjat::Module * Reinstall::IsoBuilder::Module::Factory() {

		/// @brief Proxy for legacy config files.
		class UDJAT_PRIVATE Proxy : public Udjat::Factory {
		private:
			Udjat::Factory &target ;
	
		public:
			Proxy(const char *name, Udjat::Factory *t) : Udjat::Factory(name,moduleinfo), target{*t} {
			}
	
			std::shared_ptr<Udjat::Abstract::Object> ObjectFactory(const Udjat::Abstract::Object &parent, const XML::Node &node) override {
				Logger::String{"Parsing obsolete node '",node.name(),"'"}.warning(node.attribute("name").as_string("node"));
				return target.ObjectFactory(parent,node);
			}
	
		};
		
	
		class Module : public Reinstall::IsoBuilder::Module {
		private:
			Proxy legacy{"network-installer",this};

		public:
			Module(const char *name) : Reinstall::IsoBuilder::Module(name) {
			}
			
			virtual ~Module() {
			}

		};

		return new Module("netinstall");

	}


 }
