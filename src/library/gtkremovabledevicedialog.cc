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
  * @brief Implements gtk device selection dialog.
  */

 // https://fossies.org/linux/gtkmm/demos/gtk-demo/example_dropdown.cc

 #include <config.h>
 #include <udjat/defs.h>

 #include <private/gtkremovabledevicedialog.h>
 #include <gtkmm.h>

 #include <udjat/tools/intl.h>
 #include <udjat/tools/configuration.h>

 using namespace Udjat;
 using namespace std;

#ifdef USE_MESSAGE_DIALOG

 GtkRemovableDeviceDialog::GtkRemovableDeviceDialog() : Gtk::MessageDialog{"",false,Gtk::MessageType::QUESTION,Gtk::ButtonsType::NONE} {

	setup();

	add_action_widget(cancel,-1);
	add_action_widget(dropdown,1);

	cancel.signal_clicked().connect([&]{
		close();
		response(-1);
	});
	set_message(
		Config::Value<string>{
			"messages",
			"insert-device-message",
			_("Insert an storage device <b>NOW</b> ")
		},
		true
	);

	set_secondary_text(
		Config::Value<string>{
			"messages",
			"insert-device-body",
			_("This action will <b>DELETE ALL CONTENT</b> on the device.")
		},
		true
	);

 }
#else
 GtkRemovableDeviceDialog::GtkRemovableDeviceDialog() {
	setup();

#				Gtk::Box view{Gtk::Orientation::VERTICAL};
	view.set_hexpand(true);
	view.set_vexpand(true);

	Gtk::Box contents{Gtk::Orientation::VERTICAL};
	contents.set_hexpand(true);
	contents.set_vexpand(true);
	contents.set_spacing(6);
	contents.set_margin(12);

	{
		Gtk::Label label;
		label.set_markup(
			Config::Value<string>{
				"messages",
				"insert-device-message",
				_("Insert an storage device <b>NOW</b> ")
			}.c_str()
		);
		label.get_style_context()->add_class("dialog-title");
		label.set_vexpand(false);
		contents.append(label);
	}

	{
		Gtk::Label label;
		label.set_markup(
			Config::Value<string>{
				"messages",
				"insert-device-body",
				_("This action will <b>DELETE ALL CONTENT</b> on the device.")
			}.c_str()
		);
		label.get_style_context()->add_class("dialog-subtitle");
		label.set_vexpand(false);
		contents.append(label);
	}

	view.append(contents);

	{
		Gtk::Box action_box{Gtk::Orientation::HORIZONTAL};
		action_box.get_style_context()->add_class("dialog-action-box");

		Gtk::Box buttons{Gtk::Orientation::HORIZONTAL};
		buttons.get_style_context()->add_class("dialog-action-area");
		buttons.set_hexpand(true);
		buttons.set_vexpand(false);
		buttons.set_homogeneous(true);

		cancel.set_hexpand(true);
		cancel.get_style_context()->add_class("text-button");
		cancel.set_use_underline(true);

		apply.set_hexpand(true);
		apply.get_style_context()->add_class("text-button");
		apply.set_use_underline(true);

		buttons.append(cancel);
		buttons.append(apply);

		action_box.append(buttons);
		view.append(action_box);
	}

	set_child(view);
}
#endif // USE_MESSAGE_DIALOG

 void GtkRemovableDeviceDialog::setup() {

	gtk_window_set_transient_for(
		GTK_WINDOW(gobj()),
		gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
	);

	set_modal(true);
	set_default_size(600,-1);


#ifdef USE_DROPDOWN
	auto expression = Gtk::ClosureExpression<Glib::ustring>::create(
		[](const Glib::RefPtr<Glib::ObjectBase>& item)->Glib::ustring
		{
			return std::dynamic_pointer_cast<DeviceHolder>(item)->description;
		});

	dropdown.set_expression(expression);
	dropdown.set_enable_search();

	store->append(DeviceHolder::create(DeviceHolder::AutoDetect,"",_("Waiting for device")));

#endif // USE_DROPDOWN

 }
