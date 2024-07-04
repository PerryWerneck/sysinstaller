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
  * @brief Declare abstract builder.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/progress.h>
 #include <reinstall/tools/datasource.h>
 #include <vector>
 #include <memory>
 #include <list>

 namespace Reinstall {

	class UDJAT_API Builder {
	private:
		std::vector<std::shared_ptr<Reinstall::DataSource>> sources;
		std::vector<std::shared_ptr<Reinstall::Template>> templates;

		/// @brief Append datasource in list, check for tempalte.
		void push_back(std::list<std::shared_ptr<DataSource>> &files, std::shared_ptr<DataSource> value);

	protected:
		Udjat::Dialog output;
		const Udjat::Abstract::Object &parent;

	public:
		Builder(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node);

		void prepare(Udjat::Dialog::Progress &progress, std::list<std::shared_ptr<DataSource>> &files);

	};

 }
