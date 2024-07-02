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
  * @brief Declares a reinstall data source.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/url.h>
 #include <udjat/ui/progress.h>
 #include <memory>
 #include <vector>

 #include <reinstall/tools/datasource.h>

 namespace Reinstall {

	class Repository;

	class UDJAT_API DataSource : public Udjat::NamedObject {
	protected:

		std::shared_ptr<Repository> repository;

		struct {
			const char *local = "";
			const char *remote = "";
		} url;

	public:
		DataSource(const Udjat::XML::Node &node);
		virtual ~DataSource();

		Udjat::URL local() const;
		Udjat::URL remote() const;

		void save(Udjat::Dialog::Progress &progress, const char *path);
		std::string save(Udjat::Dialog::Progress &progress);

		static bool for_each(const Udjat::URL &url, const std::function<bool(const DataSource &value)> &func);

		bool for_each(Udjat::Dialog::Progress &progress, const std::function<bool(const DataSource &value)> &func) const;

		static void load(const Udjat::XML::Node &node, std::vector<DataSource> &sources);

	};

 }
