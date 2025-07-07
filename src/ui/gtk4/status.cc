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
  * @brief Implements gtk4 progress dialog.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <glib/gi18n.h>
 #include <udjat/tools/logger.h>
 #include <string>
 #include <private/toplevel.h>

 #include <glib/gi18n.h>

 #pragma GCC diagnostic ignored "-Wattributes"
 #include <gtkmm.h>

 using namespace Udjat;
 using namespace std;

 TopLevel::Status::Status() : progress{std::make_shared<Progress>()} {

	set_valign(Gtk::Align::CENTER);
	set_hexpand(true);
	set_vexpand(false);
	set_column_spacing(3);
	set_row_spacing(3);
	set_row_homogeneous(false);
	set_column_homogeneous(false);
	set_margin(12);
	get_style_context()->add_class("dialog-contents");

	subtitle.set_valign(Gtk::Align::START);

	side_icon.get_style_context()->add_class("dialog-icon");
	side_icon.set_hexpand(false);
	side_icon.set_vexpand(false);
	// icon.set_icon_size(::Gtk::IconSize::LARGE);
	side_icon.set_pixel_size(45);
	side_icon.set_from_icon_name("logo");

	attach(main,1,0,1,1);
	attach(subtitle,1,1,1,1);
	attach(side_icon,0,0,1,2);
	attach(*progress,0,2,2,1);

#ifdef DEBUG
	progress->url("The url");
	main.set_text("The message");
	subtitle.set_text("The body");
#endif // DEBUG

 }

 TopLevel::Status::~Status() {
	debug("Destroying GTK4 status dialog");
 }

 std::shared_ptr<Udjat::Dialog::Progress> TopLevel::Status::ProgressFactory() const {
	return progress;
 }

 Udjat::Dialog::Status & TopLevel::Status::title(const char *text) noexcept {
	string str{text};
	Glib::signal_idle().connect_once([this,str](){
		main.set_text(str);
	});
	return *this;
 }

 Udjat::Dialog::Status & TopLevel::Status::sub_title(const char *text) noexcept {
	string str{text};
	progress->url("");
	Glib::signal_idle().connect_once([this,str](){
		subtitle.set_text(str);
	});
	return *this;
 }

 Udjat::Dialog::Status & TopLevel::Status::icon(const char *icon_name) noexcept {
	if(icon_name && *icon_name) {
		string str{icon_name};
		Glib::signal_idle().connect_once([this,str](){
			side_icon.set_from_icon_name(str);
		});
	}	
	return *this;
 }

 Udjat::Dialog::Status & TopLevel::Status::step(unsigned int current, unsigned int total) noexcept {
	progress->step(current,total);
	return *this;
 }

 Udjat::Dialog::Status & TopLevel::Status::state(const char *text) noexcept {
	Logger::String{text}.info("state");
	return *this;
 }

 Udjat::Dialog::Status & TopLevel::Status::busy(bool enable) noexcept {
	if(enable) {
		progress->url(_("Please wait..."));
	}
	return *this;
 }

 Udjat::Dialog::Status & TopLevel::Status::busy(const char *text) noexcept {
	progress->url(text);
	if(text && *text) {
		Logger::String{text}.info("status");
	}
	return *this;
 }
