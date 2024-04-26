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

 /**
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <libreinstall/iso9660.h>

 using namespace Udjat;

 namespace iso9660 {

#if UDJAT_CHECK_VERSION(1,2,0)
	Settings::Boot::Boot(const XML::Node &node) :
		catalog{XML::QuarkFactory(node,"boot-catalog","/boot/x86_64/loader/boot.cat")},
		eltorito{node}, efi{node} {

		}

	Settings::Settings(const XML::Node &node) :
		name{XML::QuarkFactory(node,"iso-name")},
		system_area{XML::QuarkFactory(node,"system-area")},
		volume_id{XML::QuarkFactory(node,"volume-id")},
		publisher_id{XML::QuarkFactory(node,"publisher-id")},
		data_preparer_id{XML::QuarkFactory(node,"data-preparer-id")},
		application_id{XML::QuarkFactory(node,"application-id",PACKAGE_STRING)},
		system_id{XML::QuarkFactory(node,"system-id","value")},
		boot{node}
	{

	}

#else
	Settings::Boot::Boot(const XML::Node &node) :
		catalog{XML::QuarkFactory(node,"boot-catalog","value","/boot/x86_64/loader/boot.cat").c_str()},
		eltorito{node}, efi{node} {

		}

	Settings::Settings(const XML::Node &node) :
		name{XML::QuarkFactory(node,"iso-name").c_str()},
		system_area{XML::QuarkFactory(node,"system-area").c_str()},
		volume_id{XML::QuarkFactory(node,"volume-id").c_str()},
		publisher_id{XML::QuarkFactory(node,"publisher-id").c_str()},
		data_preparer_id{XML::QuarkFactory(node,"data-preparer-id").c_str()},
		application_id{XML::QuarkFactory(node,"application-id","value",PACKAGE_STRING).c_str()},
		system_id{XML::QuarkFactory(node,"system-id","value").c_str()},
		boot{node}
	{

	}
#endif // UDJAT_CHECK_VERSION

 }
