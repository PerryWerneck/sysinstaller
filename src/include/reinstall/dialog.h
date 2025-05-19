/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declares abstract dialog.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <memory>
 #include <functional>
 #include <udjat/tools/xml.h>
 #include <cstdarg>

 namespace Reinstall {

	class UDJAT_API Dialog {
	protected:

		enum Option : uint8_t {
			None 					= 0x0000,
			AllowQuitApplication 	= 0x0001,
			AllowReboot				= 0x0002,
			AllowCancel				= 0x0004,
			AllowContinue			= 0x0008,

			NonInteractiveReboot	= 0x0010,
			NonInteractiveQuit		= 0x0020,

			AllowQuitContinue		= (AllowQuitApplication|AllowContinue),
			NonInteractive			= (NonInteractiveReboot|NonInteractiveQuit),
		} options = None;

	public:

		Dialog(const Udjat::XML::Node &node);
		virtual ~Dialog() = default;


	};
	
 }
