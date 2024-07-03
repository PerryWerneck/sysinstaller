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
 #include <string>

 namespace Reinstall {

	/// @brief Abstract disk device.
	class UDJAT_API Writer {
	private:

		/// @brief The active writer instance.
		static Writer *instance;

		int fd = -1;
		unsigned long long length = 0LL;

		/// @brief Device description.
		// std::string devdescr;

		/// @brief Device name.
		//std::string devname;

	protected:
		Writer();

		/// @brief Device set from command-line option.
		static std::string selected;

		/// @brief Open device
		void open(const char *device_name);

		/// @brief Allocate required space, exception if not enough.
		void allocate();

	public:
		virtual ~Writer();

		inline operator bool() const noexcept {
			return (bool) (fd != -1);
		}

		void close();

		/// @brief Set output from comand-line
		static void set_output(const char *path);

		/// @brief Get writer instance.
		static Writer & getInstance();

		/// @brief Select/detect and open device.
		virtual void open(Udjat::Dialog::Progress &progress, const Udjat::Dialog &dialog) = 0;

		/// @brief Get device length.
		/// @return The device length.
		/// @retval 0 The device length is undefined.
		unsigned long long size() const;

		/// @brief Set required device length.
		inline void size(unsigned long long length) {
			this->length = length;
		}

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

