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
  * @brief Implements linux device load.
  */

 #include <config.h>
 // http://storaged.org/doc/udisks2-api/latest/ref-dbus.html
 // https://stackoverflow.com/questions/63537158/how-to-list-all-the-removable-devices-with-dbus-and-udisks2
 // https://github.com/GNOME/glibmm/blob/master/examples/dbus/client_bus_listnames.cc
 // http://transit.iut2.upmf-grenoble.fr/doc/glibmm-2.4/reference/html/classGio_1_1DBus_1_1Proxy.html

 // https://unix.stackexchange.com/questions/61484/find-the-information-of-usb-devices-in-c

 #include <config.h>
 #include <udjat/defs.h>

 #include <private/gtkremovabledevicedialog.h>
 #include <gtkmm.h>

 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>

 using namespace Udjat;

 void GtkRemovableDeviceDialog::load_devices() {

 	debug("Loading devices");


 }

