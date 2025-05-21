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
  * @brief Declare application window.
  */

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <gtkmm.h>
 #include <memory>
 #include <reinstall/application.h>
 #include <udjat/tools/xml.h>

 class UDJAT_PRIVATE TopLevel : public Gtk::ApplicationWindow, private Reinstall::Application {
 private:

	class SideBar : public Gtk::Box {
	private:
		Gtk::Image logo;
   
	public:
	   SideBar();
   
	} sidebar;
   
 	class Label : public Gtk::Label {
	public:
	  // https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Label.html
	  Label(const char *style, const char *text);
	
	};
	
	class Button : public Gtk::Button {
	public:
	  Button(const char *style, const char *text);

	};

	Button apply{"suggested-action",_("_Apply")};
	Button cancel{"cancel-action",_("_Cancel")};

	Gtk::Box hbox{Gtk::Orientation::HORIZONTAL};
	Gtk::Box vbox{Gtk::Orientation::VERTICAL};

	Gtk::Box optionbox{Gtk::Orientation::VERTICAL};
	Gtk::Box buttons{Gtk::Orientation::HORIZONTAL};
	Gtk::ScrolledWindow viewport;

 protected:
	std::shared_ptr<Reinstall::Group> group_factory(const Udjat::XML::Node &node) override;

	void failed(const std::exception &e) noexcept override;

 public:
	TopLevel();
	~TopLevel() override;

	std::shared_ptr<Reinstall::Dialog> DialogFactory(const Udjat::XML::Node &node) override;

};

 