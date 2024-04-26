/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declare the repository object.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/url.h>
 #include <libreinstall/slpclient.h>
 #include <libreinstall/kernelparameter.h>
 #include <string>
 #include <cstring>

 namespace Reinstall {

	/// @brief Path to repository (A local or remote folder with files to be used by action).
	class UDJAT_API Repository {
	private:

		/// @brief Repository name.
		const char *name;

		/// @brief SLP settings for automatic source detection.
		const SlpClient slpclient;

		/// @brief Remote path.
		const char * remote = "";

		/// @brief Local path.
		const char * local = "";

		class KParm : public Kernel::Parameter {
		public:

			/// @brief Current value.
			const char *val = nullptr;

			/// @brief value to set when detected by slp.
			const char *slpval = "";

			KParm(const Udjat::XML::Node &node) :
#if UDJAT_CHECK_VERSION(1,2,0)
				Kernel::Parameter{Udjat::XML::QuarkFactory(node,"kernel-parameter-name")},
				slpval{Udjat::XML::QuarkFactory(node,"slp-kernel-parameter","slp:/")} {
#else
				Kernel::Parameter{Udjat::XML::QuarkFactory(node,"kernel-parameter-name").c_str()},
				slpval{Udjat::XML::QuarkFactory(node,"slp-kernel-parameter","value","slp:/").c_str()} {
#endif
			}

			void set_name(const char *name) noexcept {
				this->nm = name;
			}

			const std::string value() const override {
				return val;
			}

		} kparm;

	public:
		Repository(const Udjat::XML::Node &node);

		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(this->name,name) == 0;
		}

		inline const Kernel::Parameter & kernel_parameter() const noexcept {
			return kparm;
		}

		/// @brief Get repository URL, resolve it using SLP.
		Udjat::URL url() const;

		/// @brief Get local path for file.
		/// @param filepath The path inside repository.
		Udjat::URL path(const char *filepath) const;

		/// @brief Setup repository as kernel parameter.
		void set_kernel_parameter(const Udjat::XML::Node &node);

	};

 }
