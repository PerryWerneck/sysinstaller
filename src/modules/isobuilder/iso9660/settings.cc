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
  * @brief Implement iso 9660 image settings.
  */

 #include <config.h>
 #include <udjat/tools/xml.h>
 #include <memory>

 #ifdef HAVE_LIBISOFS

 #include <reinstall/modules/iso9660.h>

 using namespace Udjat;
 using namespace std;

 namespace iso9660 {

	Image::Settings::Settings(const Udjat::XML::Node &node) {

		std::string name{XML::AttributeFactory(node,"name").as_string("iso-9660")};

		system_area = XML::QuarkFactory(node,"system-area");

		volume_id = XML::QuarkFactory(node,"volume-id");
		if(!*volume_id) {
			volume_id = XML::QuarkFactory(node,"system-name");
		}

		publisher_id = XML::QuarkFactory(node,"publisher-id");
		data_preparer_id = XML::QuarkFactory(node,"data-preparer-id");
		application_id = XML::QuarkFactory(node,"application-id");
		system_id = XML::QuarkFactory(node,"system-id");

		boot.eltorito.enabled = XML::AttributeFactory(node,"eltorito").as_bool(boot.eltorito.enabled);
		boot.eltorito.id = XML::QuarkFactory(node,"system-name");

		boot.eltorito.image = XML::QuarkFactory(node,"eltorito-boot-image",boot.eltorito.image);
		boot.catalog = XML::QuarkFactory(node,"boot-catalog",(boot.eltorito.enabled ? boot.catalog : ""));

	}

 }
 #endif // HAVE_LIBISOFS

