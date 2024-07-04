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
  * @brief Declare abstract disk image.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <functional>
 #include <udjat/tools/url.h>
 #include <reinstall/tools/datasource.h>

 namespace Reinstall {

	namespace Abstract {

		/// @brief Abstract disk image.
		class UDJAT_API Image {
		protected:

			const char *name;

			constexpr Image(const char *n = "disk") : name{n} {
			}

			/// @brief Add file to image.
			/// @param from Full path for file on local file system.
			/// @param to Destination file in the image.
			virtual void append(const char *from, const char *to) = 0;

		public:
			virtual ~Image();

			/// @brief Append data source to image, download file if needed.
			void append(std::shared_ptr<DataSource> );

			/// @brief Add URL to image.
			/// @param url The URL of the file to append.
			/// @param to Destination file in the image.
			// void append(const Udjat::URL &url, const char *to);

		};

	}

 }

