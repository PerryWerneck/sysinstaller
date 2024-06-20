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
  * @brief Declare application window.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <gtkmm.h>

 #include <widgets/sidebar.h>

 #include <udjat/tools/factory.h>
 #include <udjat/tools/xml.h>

 #include <reinstall/group.h>
 #include <reinstall/action.h>

 #include <list>

 class UDJAT_PRIVATE MainWindow : public Gtk::ApplicationWindow, private Reinstall::Group::Controller {
 private:
	struct {
		SideBar sidebar;
		Gtk::Box box;
		Gtk::Box vbox{Gtk::Orientation::VERTICAL};
		Gtk::Label title{ _( "Select option" ), Gtk::Align::START };
		Gtk::Box contents{Gtk::Orientation::VERTICAL};	///< @brief The box with the groups & options.
		Gtk::ScrolledWindow swindow;
	} layout;

	struct {
		Gtk::Button apply{_("_Apply"), true}, cancel{_("_Cancel"), true};
		Gtk::Box box{Gtk::Orientation::HORIZONTAL,6};
	} button;

	class Item : public Gtk::ToggleButton {
	private:
		std::shared_ptr<Udjat::Abstract::Object> action;
		Gtk::Grid grid;
		Gtk::LinkButton help_button;
		Gtk::Label label, body;

	public:
		Item(const Udjat::XML::Node &node, std::shared_ptr<Udjat::Abstract::Object> action);
		virtual ~Item();

		inline operator bool() const noexcept {
			return (bool) action.get();
		}

		inline auto title() const noexcept {
			return label.get_text();
		}
	};

	Item *active = nullptr;
	std::list<std::shared_ptr<Item>> itens;

	class Group : public Gtk::Grid, public Reinstall::Group {
	private:
		Gtk::Label title;
		Gtk::Label sub_title;
		Gtk::Box contents{Gtk::Orientation::VERTICAL};	///< @brief The box with the options.

	public:
		Group(const Udjat::XML::Node &node);

		// Udjat::Abstract::Object
		void push_back(const Udjat::XML::Node &node, std::shared_ptr<Udjat::Abstract::Object> child) override;

	};

	static MainWindow *instance;

	static MainWindow & getInstance() {
		return *instance;
	}

 public:

	MainWindow(Glib::RefPtr<::Gtk::Application> app);
	virtual ~MainWindow();

	std::shared_ptr<Reinstall::Group> get(const Udjat::XML::Node &node) override;

 };






