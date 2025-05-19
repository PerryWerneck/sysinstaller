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
 #include <udjat/tools/factory.h>
 #include <reinstall/group.h>
 #include <memory>
 #include <unordered_map>
 #include <string>

 namespace Reinstall {

	class UDJAT_PRIVATE Application : protected Udjat::Factory {
	protected:

		/// @brief The groups.
		std::unordered_map<std::string, std::shared_ptr<Reinstall::Group>> groups;

		/// @brief Build a new group.
		virtual std::shared_ptr<Reinstall::Group> group_factory(const Udjat::XML::Node &node) = 0;

		/// @brief Notify the user that the operation has failed.
		/// @param e The exception that was thrown.
		virtual void failed(const std::exception &e) noexcept = 0;

	public:
		Application();
		virtual ~Application();

		/// @brief Load options from XML files.
		void load_options();

		bool NodeFactory(const Udjat::XML::Node &node) override;

	};

 }

