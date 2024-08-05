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

		const char *message = "";

		std::shared_ptr<Repository> repository;

		/// @brief When true allways check file timestamp with remote server.
		bool update_from_remote = true;

		const char * PathFactory(const Udjat::XML::Node &node, const char *attrname, bool required = true) const;

		DataSource() {
		}

		DataSource(const DataSource &src);

	public:

		DataSource(const char *name) : Udjat::NamedObject(name) {}

		DataSource(const Udjat::XML::Node &node);
		virtual ~DataSource();

		/// @brief Get path for source on local filesystem.
		virtual const char * local() const = 0;

		/// @brief Get path for source on remote filesystem.
		virtual const char * remote() const = 0;

		virtual bool has_local() const noexcept;

		/// @brief Get path for source on target image.
		virtual const char * path() const;

		virtual void save(Udjat::Dialog::Progress &progress, const char *path);

		/// @brief Save source, expand URL properties.
		/// @return Path to local file.
		virtual std::string save(const Udjat::Abstract::Object &object, Udjat::Dialog::Progress &progress);

		/// @brief Save source.
		virtual std::string save(Udjat::Dialog::Progress &progress);
		virtual void save(Udjat::Dialog::Progress &progress, const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer);

		static bool for_each(const Udjat::URL &url, const std::function<bool(const DataSource &value)> &func);

		bool for_each(const std::function<bool(const char *filename)> &func) const;
		bool for_each(Udjat::Dialog::Progress &progress, const std::function<bool(std::shared_ptr<DataSource> value)> &func) const;

		static void load(const Udjat::XML::Node &node, std::vector<std::shared_ptr<DataSource>> &sources, const char *nodename = nullptr);

		Udjat::URL url_local() const;
		Udjat::URL url_remote() const;

	};

	/// @brief File based data source
	class UDJAT_API FileSource : public DataSource {
	protected:
		struct {
			const char *local = "";			///< @brief The URL for source in the local filesystem.
			const char *remote = "";		///< @brief The URL for source in the remote server.
			const char *path = "";			///< @brief Path for the file inside the destination image.
		} url;

		FileSource() {
		}

	public:
		FileSource(const char *path);
		FileSource(const Udjat::XML::Node &node);
		FileSource(const Udjat::XML::Node &node, const char *nodename, bool required = true);

		virtual ~FileSource();

		bool has_local() const noexcept override;

		// DataSource
		const char * local() const override;
		const char * remote() const override;
		const char * path() const override;

		// std::string save(const Udjat::Abstract::Object &object, Udjat::Dialog::Progress &progress) override;
		//void save(Udjat::Dialog::Progress &progress, const char *path) override;

	};

 }

 namespace std {

	template <>
	struct hash<Reinstall::DataSource> {
		inline size_t operator()(const Reinstall::DataSource &obj) const {
			return std::hash<const char *>{}(obj.name());
		}
	};

	template <>
	struct equal_to<Reinstall::DataSource> {
		inline int operator()(const Reinstall::DataSource &lhs,const Reinstall::DataSource &rhs) const {
			return strcasecmp(lhs.name(),rhs.name()) == 0;
		}
	};

 }

