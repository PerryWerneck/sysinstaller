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
  * @brief Declare application main window.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/ui/gtk/userinterface.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/menu.h>

 class UDJAT_PRIVATE MainWindow : public Gtk::Window, private Udjat::UserInterface, private Udjat::Menu::Controller, private Udjat::Dialog::Controller {
 private:

 public:
 	MainWindow();
 	~MainWindow();

	void push_back(Udjat::Menu::Item *item, const XML::Node &node) override;
	void remove(const Udjat::Menu::Item *item) override;

 }
