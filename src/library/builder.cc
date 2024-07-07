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
 #include <udjat/tools/logger.h>
 #include <vector>
 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Builder::Builder(const Udjat::Abstract::Object &p, const Udjat::XML::Node &node)
		: output{"select-device",node}, parent{p} {

		{
			// Search for EFI Boot definitions
			for(auto parent = node;(parent && !boot.efi); parent = parent.parent()) {

				for(auto child = node.child("efi-boot-image");child;child = child.next_sibling("efi-boot-image")) {
					boot.efi = make_shared<EFIBootImage>(child);
					break;
				}
			}

			if(!boot.efi) {
				Logger::String{"Using default EFI Boot image"}.trace(parent.name());
				boot.efi = make_shared<EFIBootImage>();
			} else {
				Logger::String{"Using customized EFI Boot image"}.trace(parent.name());
			}
		}

		boot.theme = XML::StringFactory(node,"boot-theme");

		static const char *labels[] = {
			"grub-label",
			"boot-label",
			"system-name",
			"label",
			"title"
		};

		for(const char *label : labels) {
			boot.label = XML::QuarkFactory(node,label);
			if(*boot.label) {
				break;
			}
		}

		// Load sources.
		Reinstall::DataSource::load(node,sources);

		// Load templates
		Reinstall::Template::load(parent,node,templates);

		// Load kernel parameters.
		Reinstall::KernelParameter::load(node,kparms);

	}

	Builder::~Builder() {
	}

	std::shared_ptr<Reinstall::Template> Builder::tmplt(const char *filename) {

		for(auto &tmplt : templates) {
			if(*tmplt == filename) {
				return tmplt;
			}
		}

		return std::shared_ptr<Reinstall::Template>();
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

				path.local = source->local();
				path.remote = source->remote();

			}

			~TemplateSource() {
#ifndef DEBUG
				if(!filename.empty()) {
					unlink(filename.c_str());
				}
#endif // DEBUG
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
				tmplt->save(progress,parent,path);
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

	bool Builder::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"boot-label") && boot.label && *boot.label) {
			value = boot.label;
			return true;
		}

		if(!strcasecmp(key,"kernel-parameters")) {
			value = KernelParameter::join(parent,kparms);
			debug("Kernel parameters set to '",value.c_str(),"'");
			return true;
		}

		if(!strcasecmp(key,"boot-theme")) {

			if(boot.theme.empty()) {

				for(auto source : sources) {

					if(strncmp(source->path(),"/boot/",6)) {
						source->for_each([&](const char *filename){

							if(strncmp(filename,"./boot/",7)) {
								return false;
							}

							filename = strchr(filename+7,'/');
							if(!filename) {
								return false;
							}

							filename = strchr(filename+1,'/');
							if(!filename || strncmp(filename,"/themes/",8)) {
								return false;
							}

							filename += 8;
							const char *ptr = strchr(filename,'/');
							if(!ptr) {
								return false;
							}

							const_cast<Builder *>(this)->boot.theme = string{filename,(size_t)(ptr - filename)}.c_str();
							return true;

						});
						break;
					}
				}

				Logger::String{"Detected boot theme was '",boot.theme.c_str(),"'"}.trace(parent.name());

			}

			value = boot.theme;
			return !empty(value);

		}

		return false;
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

	void Builder::write(Udjat::Dialog::Progress &progress, Reinstall::Abstract::Image &image) {
		image.write(progress);
	}

 }

