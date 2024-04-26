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

 namespace Reinstall {

	namespace Abstract {

		class UDJAT_API DiskImage {
		protected:

			const char *name;

			constexpr DiskImage(const char *n = "disk") : name{n} {
			}

		public:
			virtual ~DiskImage();

			/// @brief Write image
			virtual void write(const std::function<int(const void *buff, size_t length, size_t total)> &writer) = 0;

		};

	}

 }

