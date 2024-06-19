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
  * @brief Implements internal SLP client.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>

 #include <private/slpclient.h>

 using namespace Udjat;

 namespace Reinstall {

	SLPClient::SLPClient(const Udjat::XML::Node &node) {
		service_type = XML::StringFactory(node,"slp-service-type");
		scope_list = XML::StringFactory(node,"slp-scope-list");
		filter = XML::StringFactory(node,"slp-filter");
	}

	std::shared_ptr<SLPClient> Factory(const Udjat::XML::Node &node) {

		// TODO: Avoid duplicated objects.

		return std::make_shared<SLPClient>(node);
	}

 }

