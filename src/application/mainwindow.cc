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
  * @brief Implements main window.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/factory.h>
 #include <udjat/module/info.h>

 #include <gtkmm.h>
 #include <private/mainwindow.h>

 static const Udjat::ModuleInfo moduleinfo{"Reinstall"};
 MainWindow::MainWindow(Glib::RefPtr<::Gtk::Application> app) : Gtk::ApplicationWindow{app}, Udjat::Factory{"group",moduleinfo} {
 }

 MainWindow::~MainWindow() {
 }

 bool MainWindow::generic(const pugi::xml_node &node) {
	return false;
 }
