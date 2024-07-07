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
 #include <reinstall/tools/kernelparameter.h>
 #include <vector>
 #include <memory>
 #include <list>
 #include <reinstall/image.h>
 #include <reinstall/tools/efiboot.h>

 namespace Reinstall {

	class UDJAT_API Builder {
	private:
		std::vector<std::shared_ptr<Reinstall::DataSource>> sources;
		std::vector<std::shared_ptr<Reinstall::Template>> templates;
		std::vector<std::shared_ptr<Reinstall::KernelParameter>> kparms;

		/// @brief Append datasource in list, check for tempalte.
		void push_back(std::list<std::shared_ptr<DataSource>> &files, std::shared_ptr<DataSource> value);

		struct {
			const char *label = nullptr;
			std::string theme;
			std::shared_ptr<EFIBootImage> efi;	///> @brief EFI boot image settings.
		} boot;

	protected:
		Udjat::Dialog output;
		const Udjat::Abstract::Object &parent;

		bool getProperty(const char *key, std::string &value) const;


		/// @brief Select device and write image to it.
		void write(Udjat::Dialog::Progress &progress, Reinstall::Abstract::Image &image);

	public:
		Builder(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node);
		virtual ~Builder();

		inline const char *name() const {
			return parent.name();
		}

		inline std::shared_ptr<EFIBootImage> efi() {
			return boot.efi;
		}

		/// @brief Find template from filename.
		/// @return Valid template ptr if filename should be replaced.
		std::shared_ptr<Reinstall::Template> tmplt(const char *filename);

		void prepare(Udjat::Dialog::Progress &progress, std::list<std::shared_ptr<DataSource>> &files);

	};

 }
