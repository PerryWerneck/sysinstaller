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
  * @brief Describe Gtk based dialog for detecting USB devices
  */

 #pragma once
 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <udjat/ui/dialog.h>
 #include <reinstall/tools/writer.h>

 #define USE_MESSAGE_DIALOG 1
 #define USE_DROPDOWN 1

 #ifdef USE_MESSAGE_DIALOG
 class UDJAT_PRIVATE GtkRemovableDeviceDialog : public Gtk::MessageDialog {
 private:
 #else
 class GtkRemovableDeviceDialog : public Gtk::Window {
 private:
 #endif // USE_MESSAGE_DIALOG

 #ifdef USE_DROPDOWN
	class DeviceHolder : public Glib::Object {
	public:

		enum Type : uint8_t {
			Undefined,
			AutoDetect,	///<< @brief Waiting for device.
			FileDialog,	///<< @brief File dialog.
			File,		///<< @brief File name.
			Device		///<< @brief Standard device entry.
		} type = Undefined;

		Glib::ustring	description;
		std::string		device_name;

		static Glib::RefPtr<DeviceHolder> create(Type type, const char *devname, const char *descr) {
			return Glib::make_refptr_for_instance<DeviceHolder>(new DeviceHolder(type, devname, descr));
		}

	private:

		DeviceHolder(Type t, const char *name, const char *descr)
			: type{t}, description{descr}, device_name{name} {
		}

	};

	/// @brief Dropdown contents.
	Glib::RefPtr<Gio::ListStore<DeviceHolder>> store = Gio::ListStore<DeviceHolder>::create();
	Gtk::DropDown dropdown{store};

 #endif // USE_DROPDOWN

	/// @brief Cancel button.
	Gtk::Button cancel{"_Cancel",true};
	Gtk::Button apply{"C_ontinue",true};

	void setup(bool allow_output_to_file);
	void load_devices();

	/// @brief Select filename for output.
	void select_file();

	/// @brief The writer
	Reinstall::Writer &writer;

 public:
	GtkRemovableDeviceDialog(Reinstall::Writer &writer, const Udjat::Dialog &dialog, bool allow_output_to_file = true);

	/// @brief Get selected device description.
	const char *description() const;

	/// @brief Get Device path
	const char *device() const;

 };

