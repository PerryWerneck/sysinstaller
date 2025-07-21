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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/action.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/writer.h>
 #include <reinstall/tools/template.h>
 #include <reinstall/tools/builder.h>
 #include <udjat/ui/status.h>
 #include <reinstall/ui/progress.h>
 #include <vector>
 #include <reinstall/modules/isobuilder.h>
 #include <list>
 #include <reinstall/modules/iso9660.h>
 #include <reinstall/modules/fatfs.h>
 #include <udjat/ui/status.h>
 #include <reinstall/application.h>

 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	static const Udjat::ModuleInfo moduleinfo{
          "Build customized installation image."
	};

	/// @brief Base class for actions.
	class UDJAT_PRIVATE IsoBuilder::Module::Action : public Reinstall::Builder {
	protected:
		virtual void build(list<std::shared_ptr<DataSource>> &files) = 0;

	public:
		Action(const Udjat::XML::Node &node)
			: Reinstall::Builder{node} {

		}

		inline const char *name() const noexcept {
			return Reinstall::Action::name();
		}

		void activate() override {

			list<std::shared_ptr<DataSource>> files;
			prepare(files);
			build(files);

			debug("IsoBuild action is complete");
			
		}

	};

	/// @brief ISO9660 builder.
#ifdef HAVE_LIBISOFS
	class UDJAT_PRIVATE Iso9660Builder : public IsoBuilder::Module::Action {
	private:
		iso9660::Image::Settings imgdef;

	protected:
		void build(list<std::shared_ptr<DataSource>> &files) override {

			// Build image ...
			auto &status = Udjat::Dialog::Status::getInstance();

			Logger::String{"Building ISO-9660 Image"}.info(name());
			status.sub_title(_("Building ISO-9660 Image"));
			status.state(_("Preparing image"));
	
			iso9660::Image image{*output,*this,imgdef};

			image.pre(*this);
			image.append(files);
			
			image.post(*this);

			// ... and write it to device.
			debug("Complete, writing...");
			image.write();
		}

	public:

		Iso9660Builder(const Udjat::XML::Node &node)
			: Action{node}, imgdef{node} {
		}


	};
#endif // HAVE_LIBISOFS

	/// @brief Fat builder.
	class UDJAT_PRIVATE FatBuilder : public IsoBuilder::Module::Action {
	private:
		FatFS::Image::Settings imgdef;

	protected:
		void build(list<std::shared_ptr<DataSource>> &files) override {

			// Build image ...
			auto &status = Udjat::Dialog::Status::getInstance();

			Logger::String{"Building Fat Image"}.info(name());
			status.sub_title(_("Building FAT Image"));

			FatFS::Image image{*output,*this,imgdef};

			image.pre(*this);
			image.append(files);
			image.post(*this);

			// ... and write it to device.
			debug("Complete, writing...");
			image.write();

		}

	public:

		FatBuilder(const Udjat::XML::Node &node)
			: Action{node}, imgdef{node} {
		}


	};

	Reinstall::IsoBuilder::Module::Module(const char *name) : Udjat::Module(name,moduleinfo), Udjat::XML::Parser{name} {
	}

	Reinstall::IsoBuilder::Module::~Module() {
	}

	bool Reinstall::IsoBuilder::Module::parse(const Udjat::XML::Node &node) {
		try {

			auto attr = XML::AttributeFactory(node,"filesystem");

#ifdef HAVE_LIBISOFS
			if(strcasecmp(attr.as_string("iso9660"),"iso9660") == 0) {
				Reinstall::Application::getInstance().push_back(
					node,
					make_shared<Iso9660Builder>(node)
				);
				return true;
			}
#endif // HAVE_LIBISOFS

			if(strcasecmp(attr.as_string("fat"),"fat") == 0 || strcasecmp(attr.as_string("fat32"),"fat32") == 0) {
				Reinstall::Application::getInstance().push_back(
					node,
					make_shared<FatBuilder>(node)
				);
				return true;
			}

			Logger::String{"Unexpected value for attribute filesystem: '",attr.as_string(),"'"}.error(XML::Parser::name());

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(XML::Parser::name());

		}

		return false;

	}

	Udjat::Module * Reinstall::IsoBuilder::Module::Factory(const char *name) {
		return new Module(name);
	}


 }
