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
  * @brief Declare an image template.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <vector>

 namespace Reinstall {

	class UDJAT_API Template : public Udjat::NamedObject {
	public:

		enum Type : uint8_t {
			Text		= 0x01,		///< @brief Template is a simple text file.
			Script		= 0x02,		///< @brief Template is a script (set 'exec' attribute).
			Binary		= 0x04,		///< @brief Template is a binary file, just replace it.
		};

		Template(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node, Type type = Template::Text);
		virtual ~Template();

		bool getProperty(const char *key, std::string &value) const override;

		static void load(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node, std::vector<Template> &templates);

	private:

		/// @brief The parent object (for properties).
		const Udjat::Abstract::Object &parent;

		Type type = (Type) 0;
		bool escape = false;
		char marker = '%';

		const char *url = nullptr;
		const char *path = nullptr;

		std::string tempfilename;	///< @brief The temporary file with template data.

	};

 }
