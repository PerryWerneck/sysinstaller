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

 TopLevel::Progress::Progress() {

	debug("Building GTK4 progress dialog");

	// Setup grid
	set_hexpand(true);
	set_vexpand(false);
	set_column_spacing(3);
	set_row_spacing(3);
	get_style_context()->add_class("dialog-footer");
	set_valign(Gtk::Align::END);

	// Setup bar
	bar.get_style_context()->add_class("dialog-progress-bar");
	bar.set_hexpand(true);
	bar.set_vexpand(false);
	bar.set_valign(Gtk::Align::START);
	bar.set_halign(Gtk::Align::FILL);
	bar.set_show_text(true);
	bar.set_ellipsize(Pango::EllipsizeMode::START);

	// Add widgets to grid
	attach(bar,0,0,2,1);
	attach(left,0,1,1,1);
	attach(right,1,1,1,1);

#ifdef DEBUG
	bar.set_text("Progress bar");
	left.set_text("left");
	right.set_text("right");
#endif // DEBUG

	timer = Glib::TimeoutSource::create(100);

	timer->connect([this]{

		if(changed) {

			idle = 0;
			changed = false;

			if(total) {
				double fraction =  ((double) current) / ((double) total);
				if(fraction > 1.0) {
					bar.set_fraction(1.0);
				} else {
					bar.set_fraction(fraction);
				}

				right.set_text(Logger::Message{_("{} of {}"),
							String{""}.set_byte((unsigned long long) current).c_str(),
							String{""}.set_byte((unsigned long long) total).c_str()
						}.c_str());

			} else {
				idle = 1000;
				right.set_text("");
			}

		} else if(idle >= 100) {

			bar.pulse();

		} else {

			idle++;

		}

		return true;

	});

	timer->attach(Glib::MainContext::get_default());

}

TopLevel::Progress::~Progress() {
	debug("Destroying GTK4 progress dialog");
	timer->destroy();
}

Udjat::Dialog::Progress & TopLevel::Progress::step(const unsigned int current, const unsigned int total) noexcept {
	Glib::signal_idle().connect([this,current,total](){
		if(total) {
			left.set_text(Logger::Message{_("{} of {}"), current, total}.c_str());
		} else {
			left.set_text("");
		}
		right.set_text("");
		idle = 1000;
		return 0;
	});
	return *this;
}

Udjat::Dialog::Progress & TopLevel::Progress::set(uint64_t current, uint64_t total, bool) noexcept {
	this->current = current;
	this->total = total;
	changed = true;
	return *this;
}

Udjat::Dialog::Progress & TopLevel::Progress::url(const char *url) noexcept {
	string u{url};
	Glib::signal_idle().connect_once([this,u](){
		idle = 1000;
		changed = true;
		bar.set_text( u.empty() ? _("Wait...") : u );
	});
	return *this;
}

