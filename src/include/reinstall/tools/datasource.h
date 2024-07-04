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
	class Template;

	class UDJAT_API DataSource : public Udjat::NamedObject {
	protected:

		std::shared_ptr<Repository> repository;

		/// @brief When true allways check file timestamp with remote server.
		bool update_from_remote = true;

		const char * PathFactory(const Udjat::XML::Node &node, const char *attrname) const;
		Udjat::URL UrlFactory(const char *relative) const;

	public:

		DataSource(const char *name) : Udjat::NamedObject(name) {}

		DataSource(const Udjat::XML::Node &node);
		virtual ~DataSource();

		/// @brief Get URL for source on local filesystem.
		virtual const char * local() const = 0;

		/// @brief Get URL for source on remote filesystem.
		virtual const char * remote() const = 0;

		/// @brief Get path for source on target image.
		virtual const char * path() const;

		void save(Udjat::Dialog::Progress &progress, const char *path);
		std::string save(Udjat::Dialog::Progress &progress);

		static bool for_each(const Udjat::URL &url, const std::function<bool(const DataSource &value)> &func);

		bool for_each(Udjat::Dialog::Progress &progress, std::vector<Template> &templates, const std::function<bool(std::shared_ptr<DataSource> value)> &func) const;

		static void load(const Udjat::XML::Node &node, std::vector<std::shared_ptr<DataSource>> &sources);

	};

	/// @brief Data source with remote and local URLs
	class UDJAT_API FileSource : public DataSource {
	private:
		struct {
			const char *local = "";		///< @brief The URL for source in the local filesystem.
			const char *remote = "";	///< @brief The URL for source in the remote server.
		} url;

	public:
		FileSource(const Udjat::XML::Node &node);

		// DataSource
		const char * local() const override;
		const char * remote() const override;

	};

 }
