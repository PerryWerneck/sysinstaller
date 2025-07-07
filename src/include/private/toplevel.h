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
 #include <reinstall/dialog.h>
 #include <udjat/tools/xml.h>
 #include <reinstall/tools/writer.h>
 #include <reinstall/action.h>
 #include <udjat/ui/progress.h>
 #include <udjat/ui/status.h>
 #include <udjat/ui/progress.h>

 class UDJAT_PRIVATE TopLevel : public Gtk::ApplicationWindow, protected Reinstall::Application, private Reinstall::Writer {
 private:
 public:
	TopLevel();
	~TopLevel() override;

 protected:

  	class Label : public Gtk::Label {
	public:
	  // https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Label.html
	  Label(const char *style, const char *text, Gtk::Align align = Gtk::Align::START);
	
	};
	
	class Button : public Gtk::Button {
	public:
	  Button(const char *style, const char *text);

	};

	class Progress : public Gtk::Grid, public Udjat::Dialog::Progress {
	private:	
		Gtk::ProgressBar bar;
		Label left{"dialog-left-label","",Gtk::Align::START};
		Label right{"dialog-right-label","",Gtk::Align::END};

		Glib::RefPtr<Glib::TimeoutSource> timer;
		unsigned int idle = (unsigned int) -1;

		bool changed = false;
		uint64_t current = 0;
		uint64_t total = 0;

	public:
		Progress();
		~Progress() override;

		Udjat::Dialog::Progress & step(const unsigned int current = 0, const unsigned int total = 0) noexcept override;
			
		Udjat::Dialog::Progress & set(uint64_t current, uint64_t total, bool) noexcept override;

		Udjat::Dialog::Progress & url(const char *url) noexcept override;

	};
	
	/// @brief Start XML parsing
	void start();

	void failed(const std::exception &e) noexcept override;

	/// @brief Open target device for writing.
	/// @see Reinstall::Writer::open()
	void open(const Reinstall::Dialog &settings) override;

	std::shared_ptr<Reinstall::Dialog> DialogFactory(const char *name, const Udjat::XML::Node &node, const char *message, const Reinstall::Dialog::Option option) override;	

 };

 class UDJAT_PRIVATE NonInteractiveWindow : public TopLevel, private Udjat::Dialog::Status {
 private:
	std::shared_ptr<Reinstall::Action> action;
	Progress progress;

 public:
	NonInteractiveWindow();
	~NonInteractiveWindow() override;

	std::shared_ptr<Reinstall::Group> group_factory(const Udjat::XML::Node &node) override;

	void activate() noexcept override;

 };

 class UDJAT_PRIVATE InteractiveWindow : public TopLevel {
 private:

	class Item;

	class SideBar : public Gtk::Box {
	private:
		Gtk::Image logo;
   
	public:
	   SideBar();
   
	} sidebar;
   
	Button apply;
	Button cancel;

	Gtk::Box hbox{Gtk::Orientation::HORIZONTAL};
	Gtk::Box vbox{Gtk::Orientation::VERTICAL};

	Gtk::Box optionbox{Gtk::Orientation::VERTICAL};
	Gtk::Box buttons{Gtk::Orientation::HORIZONTAL};
	Gtk::ScrolledWindow viewport;

 public:
	InteractiveWindow();
	~InteractiveWindow() override;

 protected:
	std::shared_ptr<Reinstall::Group> group_factory(const Udjat::XML::Node &node) override;

	void select(std::shared_ptr<Reinstall::Action> action) override;

	void activate() noexcept override;

 };

 