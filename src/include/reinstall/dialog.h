/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <vector>

 namespace Reinstall {

	class UDJAT_API Dialog {
	public:

		enum Option : uint8_t {
			None 					= 0x00,
			Quit 					= 0x01,
			Reboot					= 0x02,
			Cancel					= 0x04,
			Continue				= 0x08,

			QuitContinue 			= Quit|Continue,		///< Quit or continue.
			NonInteractive	 		= 0x10,
			NonInteractiveQuit 		= 0x30,					///< Force quit without asking.
			NonInteractiveReboot 	= 0x50,					///< Force reboot without asking.
			AutoConfirm				= 0x80,					///< Automatically confirm the dialog.	
		};

		static Option OptionFactory(const char *name);

		Dialog() = default;

		Dialog(const Udjat::XML::Node &node, const char *message = "", const Option option = None);
		~Dialog() = default;

		static std::shared_ptr<Dialog> Factory(const char *name, const Udjat::XML::Node &node, const char *message = "", const Option option = None);

		void set(const Option option);
		static void preset(const Option option) noexcept;

		/// @brief Check option.
		inline bool has(const Option option) const noexcept {
			return (options & option) == option;
		}

		/// @brief Ask for confirmation.
		virtual bool ask(bool default_response = true) const noexcept;
		
		/// @brief Show the dialog without any message.
		/// @return true if an action was taken, false otherwise.
		virtual bool present(const char *msg = nullptr) const noexcept;

		/// @brief Show the dialog with an error message.
		/// @param e The exception to show.
		inline void present(const std::exception &e) const {
			present(e.what());
		}

		const char *text(const char *def = "") const noexcept;
		const char *body(const char *def = "") const noexcept;

		virtual void quit() const;
		virtual void cancel() const;
		virtual void reboot() const;

	protected:

		static Option presets;

		/// @brief The dialog options.
		/// @note The options are set by the XML file.
		Option options = None;

		/// @brief The button options.
		struct Buttons {
			std::vector<Option> order;	///< The button order.

			Option destructive = None;	///< The destructive button option.
			Option suggested = None; 	///< The suggested button option.

			Buttons() = default; 
			Buttons(const Udjat::XML::Node &node);
		} buttons;
		
		/// @brief The title for the dialog (not the title bar).
		/// @note This is the title visible in bold inside the dialog, usually the same as the menu item.
		const char *title;

		/// @brief The message to show in the dialog.
		const char *message = nullptr;

		/// @brief The message to show in the dialog.
		/// @note This is the message shown in the dialog, usually a description of the action.
		const char *details = nullptr;

		/// @brief Is this popup destructive?
		/// @note This is used to show a warning icon in the dialog.
		bool destructive = false;

	};
	
 }
