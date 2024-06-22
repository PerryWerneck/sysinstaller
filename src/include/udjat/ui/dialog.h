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
  * @brief Declares abstract dialog.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <memory>
 #include <functional>
 #include <udjat/tools/xml.h>
 #include <cstdarg>

 namespace Udjat {

	class UDJAT_API Dialog {
	protected:

		struct {
			const char *name = nullptr;
			const char *icon_name = nullptr;
			const char *message = nullptr;
			const char *details = nullptr;
		} args;

		Dialog();

	public:
		/// @brief Declare dialog from XML Node.
		Dialog(const XML::Node &node, const char *name);

		inline void message(const char *message) noexcept {
			args.message = message;
		}

		inline const char * message() const noexcept {
			return args.message;
		}

		inline void details(const char *details) noexcept {
			args.details = details;
		}

		inline const char * details() const noexcept {
			return args.details;
		}

		inline operator bool() const noexcept {
			return (bool) args.message && *args.message;
		}

		class Popup;
		class Progress;

		class UDJAT_API Controller {
		public:
			Controller();
			virtual ~Controller();

			static Controller & getInstance();

			virtual std::shared_ptr<Popup> PopupFactory() = 0;
			virtual std::shared_ptr<Progress> ProgressFactory() = 0;
			virtual int select(const Dialog &dialog, int cancel, const char *button, va_list args) noexcept = 0;

		};

		friend class Controller;

		virtual ~Dialog();

		/// @brief Get confirmation message.
		bool confirm() const noexcept;

		int select(int cancel, const char *button, ...) const __attribute__ ((sentinel));

	};

 }

