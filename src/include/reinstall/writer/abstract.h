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
  * @brief Declares abstract disk writer.
  */

 #pragma once
 namespace Reinstall {

	namespace Abstract {

		/// @brief Abstract disk device.
		class UDJAT_API Writer {
		public:
			Writer();
			virtual ~Writer();

			/// @brief Get device length.
			/// @return The device length.
			/// @retval 0 The device length is undefined.
			virtual unsigned long long size() const;

			/// @brief Write data to device.
			/// @param offset Offset of current block.
			/// @param length Total size of image (0 = undefined).
			virtual void write(unsigned long long offset, const void *contents, unsigned long long length = 0) = 0;

		};

	}

 }

