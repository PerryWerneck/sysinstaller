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

 #ifdef LOG_DOMAIN
	#undef LOG_DOMAIN
 #endif // LOG_DOMAIN
 #define LOG_DOMAIN "isowriter"
 #include <udjat/tools/logger.h>

 #include <udjat/module.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/xml.h>
 #include <udjat/module/info.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/action.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/writer.h>
 #include <udjat/ui/status.h>
 #include <udjat/ui/progress.h>
 #include <reinstall/modules/isowriter.h>
 #include <reinstall/application.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	static const Udjat::ModuleInfo moduleinfo{
		"Download and write an ISO file."
  	};

	class UDJAT_PRIVATE IsoWriter::Module::Action : public Reinstall::Action {
	private:
		Reinstall::FileSource iso;
		bool use_cached = true;

	public:

		Action(const Udjat::XML::Node &node)
			: Reinstall::Action{node}, iso{node}, use_cached{node.attribute("cache").as_bool(use_cached)} {
		}

		~Action() {
		}

		void activate() override {

			auto &status = Udjat::Dialog::Status::getInstance();

			if(iso.has_local() && use_cached) {

				Logger::String{"Updating and writing local ISO image '",iso.local(),"'."}.info();

				status.sub_title(_("Updating ISO image"));
				auto path = iso.save(*this);
				status.sub_title(_("Writing ISO image"));
				Reinstall::Writer::getInstance().write(path.c_str());

			} else {

				status.sub_title(_("Getting ISO image"));
				
				auto url = iso.url_remote();	// Use remote URL for download, this method will resolve relative URLs and SLP repos.
				auto progress = iso.ProgressFactory();
				progress->url(url);

				Logger::String{"Getting ISO image from '",url,"'."}.info();

				auto filename = File::Temporary::create();

				try {

					Udjat::File::Handler file{filename.c_str(),true};
					URL{url}.get([&progress,&file](uint64_t current, uint64_t total, const void *buf, size_t length){
						file.write(buf,length);
						progress->set(current+length,total);
						return false;
					});
					progress->done();

					Logger::String{"ISO image saved to '",filename.c_str(),"'"}.trace();

					status.sub_title(_("Writing ISO image"));
					Reinstall::Writer::getInstance().write(filename.c_str());

				} catch(...) {

					unlink(filename.c_str());
					throw;
				}

			}

		}

	};
  
	Udjat::Module * IsoWriter::Module::Factory() {
		return new Module();
	}
	
	IsoWriter::Module::Module() : Udjat::Module("isowriter",moduleinfo), Udjat::XML::Parser{"iso-writer"} {
	};

	IsoWriter::Module::~Module() {
	}

	// Udjat::XML::Parser interface.
	bool IsoWriter::Module::parse(const Udjat::XML::Node &node) {
		// Logger::String{"Building action '",node.attribute("name").as_string(),"' from '",node.path(),"'"}.info();
		Reinstall::Application::getInstance().push_back(node,make_shared<IsoWriter::Module::Action>(node));
		return true;
	}

 }

