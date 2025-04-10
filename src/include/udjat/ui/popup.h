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
  * @brief Declares popup dialog.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/ui/dialog.h>

 namespace Udjat {

	class UDJAT_API Dialog::Popup {
	protected:
		Popup();

	public:
		static std::shared_ptr<Popup> Factory();

		virtual int run(const std::function<int(Popup &popup)> &task) noexcept;

		/// @brief Sets the message that will be shown in the alert.
		virtual Popup & message(const char *message);

		/// @brief Sets the detail text that will be shown in the alert.
		virtual Popup & detail(const char *text);

		virtual ~Popup();

	};

 }
