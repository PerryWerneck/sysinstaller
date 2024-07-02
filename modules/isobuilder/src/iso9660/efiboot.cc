/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <memory>
 #include <efiboot.h>

 #include <udjat/tools/xml.h>

 using namespace std;
 using namespace Udjat;

 namespace iso9660 {

	std::shared_ptr<EFIBootImage> EFIBootImage::factory(const XML::Node &node) {
		return make_shared<EFIBootImage>(node);
	}

	EFIBootImage::EFIBootImage(const pugi::xml_node &node) : NamedObject{node} {

		options.enabled = XML::AttributeFactory(node,"efi-boot-image").as_bool(options.enabled);
		options.path = XML::QuarkFactory(node,"efi-boot-image",options.path);

//		options.size = Reinstall::Action::getImageSize(node);
//		if(options.size) {
//			Logger::String{"Will build a ",String{}.set_byte((unsigned long long) options.size)," boot image"}.trace(name());
//		}

	}

	/*
	void EFIBootImage::build(Reinstall::Action &action) {

		debug("-----------------------------------------------------------------");

		if(options.size) {

			Dialog::Progress::getInstance().set_sub_title(_("Building EFI Boot image"));

			std::string imgfilename{File::Temporary::create()};

			// Create disk.
			{
				// Create disk
				Disk::Image disk{
					imgfilename.c_str(),
					Config::Value<string>{"efi","filesystem","vfat"}.c_str(),
					options.size
				};

				// Copy EFI files
				action.for_each([&disk](Source &source){

					if(strncasecmp(source.path,"efi/",4)) {
						return;
					}

					disk.copy(source.filename(),source.path);

				});
			}

			// Add it to action.
			{
				class EFIBootSource : public Reinstall::Source {
				public:
					EFIBootSource(const char *filename, const char *path) : Reinstall::Source{"efiboot","disk://efi",path} {
						filenames.temp = filenames.saved = filename;
					}
				};

				action.push_back(make_shared<EFIBootSource>(imgfilename.c_str(),options.path));

			}


		}

	}
	*/

 }
