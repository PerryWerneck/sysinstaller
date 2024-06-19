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
  * @brief Declare a repository.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <memory>

 namespace Reinstall {

	class SLPClient;

	class UDJAT_API Repository : public Udjat::NamedObject {
	private:

		std::shared_ptr<SLPClient> slpclient;

		struct {
			const char *local = "";			///< @brief Local (file://) URL, has precedence.
			const char *remote = "";		///< @brief Secondary URL, the remote one.
		} url;

	public:

		Repository(const Udjat::XML::Node &node);
		virtual ~Repository();

	};

 }


