/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements abstract builder.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/tools/builder.h>
 #include <udjat/tools/intl.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/template.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/progress.h>
 #include <vector>
 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Builder::Builder(const Udjat::Abstract::Object &p, const Udjat::XML::Node &node)
		: output{"select-device",node}, parent{p} {

		// Load sources.
		Reinstall::DataSource::load(node,sources);

		// Load templates
		Reinstall::Template::load(parent,node,templates);

	}

	void Builder::push_back(std::list<std::shared_ptr<DataSource>> &files, std::shared_ptr<DataSource> value) {

		class TemplateSource : public DataSource {
		private:
			const Udjat::Abstract::Object &parent;	///< @brief Parent object (for properties).
			std::shared_ptr<Reinstall::Template> tmplt;

			std::string filename;	///< @brief The temporary file with template applyed.

			struct {
				std::string local;
				std::string remote;
			} path;

		public:
			TemplateSource(const Udjat::Abstract::Object &p, std::shared_ptr<Reinstall::Template> t, std::shared_ptr<DataSource> source)
				: DataSource{*source},parent{p},tmplt{t} {

				rename(source->name());
				path.local = source->local();
				path.remote = source->remote();

			}

			~TemplateSource() {
				if(!filename.empty()) {
					unlink(filename.c_str());
				}
			}

			/// @brief Get URL for source on local filesystem.
			const char * local() const override {
				return path.local.c_str();
			}

			/// @brief Get URL for source on remote filesystem.
			const char * remote() const override {
				return path.remote.c_str();
			}


			void save(Udjat::Dialog::Progress &progress, const char *path) override {
				Udjat::URL url = url_local();

				debug("Getting template from '",url.c_str(),"'");

			}

			std::string save(Udjat::Dialog::Progress &progress) override {
				if(filename.empty()) {
					filename = File::Temporary::create();
					save(progress,filename.c_str());
				}
				return filename;
			}

		};

		// Check for template.
		for(auto &tmplt : templates) {
			const char *remote = value->remote();
			if(*tmplt == remote) {
				Logger::String{"Using template '",tmplt->name(),"' for ",remote}.trace(parent.name());
				files.push_back(make_shared<TemplateSource>(parent,tmplt,value));
				return;
			}
		}

		files.push_back(value);
	}

	void Builder::prepare(Udjat::Dialog::Progress &progress, list<std::shared_ptr<DataSource>> &files) {

		progress = _("Getting required files");

		for(auto &source : sources) {
			source->for_each(progress,[this,&files](std::shared_ptr<DataSource> value){
				push_back(files,value);
				return false;
			});
		}

		Logger::String{files.size()," files to download"}.trace(parent.name());

	}

 }

