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
 #include <udjat/defs.h>
 #include <udjat/ui/progress.h>

 namespace Reinstall {

	/// @brief Abstract disk device.
	class UDJAT_API Writer {
	private:
		static const char *devname;
		static Writer *instance;
		int fd = -1;

	protected:
		Writer();

		/// @brief Open device
		/// @return 0 of ok, error code if not.
		int open(const char *device_name);

	public:
		virtual ~Writer();

		void close();

		/// @brief Get writer instance.
		static Writer & getInstance();

		/// @brief Select/detect and open device.
		virtual void open(Udjat::Dialog::Progress &progress, const Udjat::Dialog &dialog) = 0;

		bool allocate(unsigned long long length);

		/// @brief Get device length.
		/// @return The device length.
		/// @retval 0 The device length is undefined.
		unsigned long long size() const;

		/// @brief Write data to device.
		/// @param offset Offset of current block.
		/// @param length Block length.
		void write(unsigned long long offset, const void *contents, unsigned long long length);

		/// @brief Write iso image to device.
		void write(Udjat::Dialog::Progress &progress,const Udjat::Dialog &dialog, const char *isoname);

	};

	class UDJAT_API GtkWriter : public Writer {
	public:
		GtkWriter() : Writer() {
		}

		void open(Udjat::Dialog::Progress &progress, const Udjat::Dialog &dialog) override;

	};

 }

