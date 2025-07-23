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
  * @brief Declare an action.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <reinstall/dialog.h>
 #include <memory>
 #include <string>
 
 namespace Reinstall {

	class UDJAT_API Model {
	public:
		Model(const Udjat::XML::Node &node);

	};

	class UDJAT_API Action : private Model, public Udjat::NamedObject {
	private:
		static const char *presets[2];
		const char * dialog_title;
		const char * icon_name;

	public:

		Action(const Udjat::XML::Node &node);
		virtual ~Action();

		static bool is_default(const Udjat::XML::Node &node) noexcept;

		static void preset(const char *value);

		/// @brief Get the action title.
		inline const char *title() const noexcept {
			return dialog_title;
		}

		inline const char *icon() const noexcept {
			return icon_name;
		}

		static inline bool has_preset() noexcept {
			return (presets[0] && presets[1]);
		}

		/// @brief Activate the action, called on selected action when the 'apply' button is pressed.
		virtual void activate();

		/// @brief Test if the action is valid and can be activated.
		virtual bool initialize();

		std::shared_ptr<Dialog> confirmation;
		std::shared_ptr<Dialog> success;
		std::shared_ptr<Dialog> failed;

	};

 }

