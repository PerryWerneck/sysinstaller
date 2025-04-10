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
 #include <reinstall/tools/template.h>
 #include <udjat/ui/progress.h>
 #include <reinstall/ui/dialog.h>
 #include <list>

 namespace Reinstall {

	class Builder;

	namespace Abstract {

		/// @brief Abstract disk image.
		class UDJAT_API Image {
		protected:
			const Udjat::Dialog &dialog;
			Reinstall::Builder &builder;

			/// @brief EFI boot partition image file.
			std::string efibootpart;

			inline Image(const Udjat::Dialog &s, Reinstall::Builder &b) : dialog{s}, builder{b} {
			}

			/// @brief Add file to image.
			/// @param from Full path for file on local file system.
			/// @param to Destination file in the image.
			virtual void append(const char *from, const char *to) = 0;

			/// @brief Add sources to image.
			void append(Udjat::Dialog::Progress &progress, std::list<std::shared_ptr<DataSource>> &sources);

		public:
			virtual ~Image();

			static const char * application_id() noexcept;

			/// @brief Append data source to image, download file if needed.
			virtual void append(std::shared_ptr<DataSource> source);

			virtual void write(Udjat::Dialog::Progress &dialog, const std::function<void(unsigned long long offset, const void *contents, unsigned long long length)> &task);

			virtual void write(Udjat::Dialog::Progress &progress);

		};

	}

 }

