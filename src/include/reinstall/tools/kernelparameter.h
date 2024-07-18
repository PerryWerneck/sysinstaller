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
  * @brief Declare kernel parameter.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <memory>
 #include <vector>

 namespace Reinstall {

	class UDJAT_API KernelParameter  : public Udjat::NamedObject {
	private:

		struct Value {
			const char *name;
			const char *value;

			Value(const char *name, const char *value);

		};

		static std::vector<Value> default_values;

	protected:
		const char *refvalue = nullptr;

	public:
		constexpr KernelParameter(const char *name) : Udjat::NamedObject{name} {
		}

		KernelParameter(const Udjat::XML::Node &node, const char *attrname = "value");
		virtual ~KernelParameter();

		/// @brief Override xml defined kernel parameters.
		static inline void insert_default(const char *name, const char *value) {
			default_values.emplace_back(name,value);
		}

		static void insert_default(const char *arg);

		/// @brief Apply properties
		/// @return The parameter value with object properties applied.
		virtual std::string value(const Udjat::Abstract::Object &object) const;

		static void load(const Udjat::XML::Node &node, std::vector<std::shared_ptr<KernelParameter>> &parameters);
		static std::string join(const Udjat::Abstract::Object &object, const std::vector<std::shared_ptr<KernelParameter>> &parameters);

	};

 }
