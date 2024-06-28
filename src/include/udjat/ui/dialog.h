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
	public:

		enum Option : uint8_t {
			None 					= 0x0000,
			AllowQuitApplication 	= 0x0001,
			AllowReboot				= 0x0002,
			AllowCancel				= 0x0004,
			AllowContinue			= 0x0008,

			AllowQuitContinue		= (AllowQuitApplication|AllowContinue)
		};

	protected:

		Option options = None;

		struct Args {
			const char *name = "dialog";
			const char *icon_name = nullptr;
			const char *message = nullptr;
			const char *details = nullptr;

			constexpr Args() {
			}

			constexpr Args(const char *i, const char *m, const char *d)
				: icon_name{i},message{m},details{d} { }

			constexpr Args(const char *m, const char *d)
				: message{m},details{d} { }

		} args;

	public:

		constexpr Dialog(const char *name) {
			args.name = name;
		}

		/// @brief Declare dialog from XML Node.
		Dialog(const char *name, Option options, const XML::Node &node);

		Dialog(const char *name, const XML::Node &node) : Dialog{name,Dialog::AllowContinue,node} {
		}

		inline void message(const char *message) noexcept {
			args.message = message;
		}

		inline bool test(const Option options) const noexcept {
			return this->options & options;
		}

		std::string message(const char *def = "") const;

		inline void details(const char *details) noexcept {
			args.details = details;
		}

		std::string details(const char *def = "") const;

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
			virtual void present(const Dialog &dialog) = 0;
			virtual int select(const Dialog &dialog, int cancel, const char *button, va_list args) noexcept = 0;

		};

		friend class Controller;

		virtual ~Dialog();

		/// @brief Get confirmation message.
		bool confirm() const noexcept;

		int select(int cancel, const char *button, ...) const __attribute__ ((sentinel));

		inline void present() const noexcept {
			Dialog::Controller::getInstance().present(*this);
		}

	};

 }

