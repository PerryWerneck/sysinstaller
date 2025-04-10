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
  * @brief Declare an action.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <reinstall/ui/dialog.h>
 #include <udjat/ui/progress.h>

 namespace Reinstall {

 	class Group;

	class UDJAT_API Action : public Udjat::NamedObject {
	protected:

		struct Args {
			const char *icon_name = nullptr;
			const char *title = nullptr;
			const char *sub_title = nullptr;
			const char *dialog_title = "";

			Args(const Udjat::XML::Node &node)
				: icon_name{Udjat::XML::QuarkFactory(node,"icon-name")},
				  title{Udjat::XML::QuarkFactory(node,"title")},
				  sub_title{Udjat::XML::QuarkFactory(node,"sub-title")} {
			}

		} args;

		Dialog confirmation;
		Dialog success;
		Dialog failed;

	public:

		Action(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node);
		virtual ~Action();

		void activate();

		bool getProperty(const char *key, std::string &value) const override;

		inline const char * icon_name() const noexcept {
			return args.icon_name;
		}

		inline const char * title() const noexcept {
			return args.title;
		}

		inline const char * sub_title() const noexcept {
			return args.sub_title;
		}

		virtual int activate(Udjat::Dialog::Progress &progress);

		/// @brief Test if the action is valid and can be activated.
		virtual bool initialize();

	};

 }

