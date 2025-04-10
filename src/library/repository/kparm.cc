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
  * @brief Implements Source repository.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/progress.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/configuration.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>
 #include <private/slpclient.h>
 #include <list>

 #ifdef HAVE_ZLIB
	#include <zlib.h>
 #endif // HAVE_ZLIB

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Repository::KParm::KParm(const Udjat::XML::Node &node) {
		for(auto child = node.child("attribute");child;child = child.next_sibling("attribute")) {
			if(!strcasecmp(child.attribute("name").as_string("none"),"kernel-parameter-name")) {
				name = String{child,"value"}.as_quark();
				slp = XML::QuarkFactory(node,"slp-value");
				enabled = child.attribute("allow-slp").as_bool(true);
			}
		}

		if(!(slp && *slp) && Config::Value{"application","legacy",true}) {
			slp = XML::QuarkFactory(node,"slp-kernel-parameter");
			if(slp && *slp) {
				Logger::String{"Got slp kernel parameter using legacy attribute 'slp-kernel-parameter'"}.trace(node.attribute("name").as_string("repository"));
			}
		}

	}

 }


