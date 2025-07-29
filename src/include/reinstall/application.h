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
  * @brief Declare main application.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <reinstall/group.h>
 #include <reinstall/dialog.h>
 #include <reinstall/action.h>
 #include <memory>
 #include <unordered_map>
 #include <string>
 #include <udjat/tools/xml.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/url/handler/http.h>

 namespace Reinstall {

	class Action;

	class UDJAT_PRIVATE Application : protected Udjat::XML::Parser {
	private:
		static Application *instance;	///< @brief Singleton instance.

	protected:

		/// @brief The groups.
		std::unordered_map<std::string, std::shared_ptr<Reinstall::Group>> groups;

		/// @brief Build a new group.
		virtual std::shared_ptr<Reinstall::Group> group_factory(const Udjat::XML::Node &node) = 0;

		/// @brief Notify the user that the operation has failed.
		/// @param e The exception that was thrown.
		virtual void failed(const std::exception &e) noexcept = 0;

		/// @brief The selected action.
		std::shared_ptr<Action> action;

		/// @brief Options loaded from XML files.
		virtual void loaded() noexcept;

	public:

		Application();
		virtual ~Application();

		static Application & getInstance();

		/// @brief Build a new dialog.
		/// @param node The dialog description.
		/// @return Pointer to the dialog.
		virtual std::shared_ptr<Reinstall::Dialog> DialogFactory(const char *name, const Udjat::XML::Node &node, const char *message, const Dialog::Option option = Dialog::None) = 0;	
	
		/// @brief Push-bach an action based on the XML node.
		void push_back(const Udjat::XML::Node &node, std::shared_ptr<Action> child);

		/// @brief Set the selected action.
		virtual void select(std::shared_ptr<Action> action);

		/// @brief Load options from XML files.
		void load_options();

		/// @brief Activate the selected action.
		virtual void activate() noexcept;

		bool parse(const Udjat::XML::Node &node) override;

	};

 }

