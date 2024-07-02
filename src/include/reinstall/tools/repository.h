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
 #include <vector>

 #include <reinstall/tools/datasource.h>

 namespace Reinstall {

	class SLPClient;

	class UDJAT_API Repository : public DataSource {
	private:

		std::shared_ptr<SLPClient> slpclient;

		/// @brief The repository files (from INDEX.gz)
		std::vector<std::string> files;

	public:

		static std::shared_ptr<Repository> Factory(const Udjat::XML::Node &node);

		Repository(const Udjat::XML::Node &node);
		virtual ~Repository();

		/// @brief Load and parse INDEX.gz
		bool index();

	};

 }


