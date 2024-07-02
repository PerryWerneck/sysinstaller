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

 #pragma once

 #include <udjat/defs.h>
 #include <reinstall/action.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <memory>

 namespace iso9660 {

	/// @brief The EFI Boot Image;
	class UDJAT_API EFIBootImage : public Udjat::NamedObject {
	private:

		struct {
			bool enabled = true;

			/// @brief Path for EFI boot inside image.
			const char *path = "/boot/x86_64/efi";

			/// @brief Size of image in bytes
			unsigned long size = 0;

			/// @brief Filesystem for image.
			enum FileSystem : uint8_t {
				FAT32
			} filesystem = FAT32;

		} options;

	public:

		EFIBootImage(const Udjat::XML::Node &node);

		static std::shared_ptr<EFIBootImage> factory(const pugi::xml_node &node);

		inline bool enabled() const noexcept {
			return options.enabled;
		}

		inline const char * path() const noexcept {
			return options.path;
		}

		/// @brief Build image (if necessary), add source to action.
		virtual void build(Reinstall::Action &action);

	};

 }
