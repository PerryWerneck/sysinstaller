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
  * @brief Declare ISO9660 disk image.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <reinstall/image.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/tools/datasource.h>

 typedef struct Iso_Image IsoImage;
 typedef struct iso_write_opts IsoWriteOpts;

 namespace iso9660 {

	class UDJAT_API Image : public Reinstall::Abstract::Image {
	public:

		/// @brief ISO9660 image definitions.
		struct Settings {

			const char *system_area = nullptr;
			const char *volume_id = nullptr;
			const char *publisher_id = nullptr;
			const char *data_preparer_id = nullptr;
			const char *application_id = nullptr;
			const char *system_id = nullptr;
			int iso_level = 3;
			int rockridge = 1;
			int joliet = 1;
			int allow_deep_paths = 1;
			bool like_iso_hybrid = true;

			struct Boot {

				const char *catalog = nullptr;

				struct ElTorito {

					bool enabled = true;
					const char *id = nullptr;
					const char *image = nullptr;

					inline operator bool() const noexcept {
						return enabled;
					}

				} eltorito;

			} boot;

			Settings(const Udjat::XML::Node &node);

		};

		Image(Reinstall::Builder *builder, std::shared_ptr<Settings> settings);
		virtual ~Image();

		void pre(Udjat::Abstract::Object &object);

		void post(Udjat::Abstract::Object &object);

		void write() override;


		inline void append(std::list<std::shared_ptr<Reinstall::DataSource>> &sources) {
			Reinstall::Abstract::Image::append(sources);
		}

	protected:
		// Abstract::Image
		void append(const char *from, const char *to) override;

	private:
		std::shared_ptr<Settings> settings;
		IsoImage *image = nullptr;
		IsoWriteOpts *opts = nullptr;

	};

 }

