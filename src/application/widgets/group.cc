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
  * @brief Implements GTK4 group widget.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>

 #include <gtkmm.h>
 #include <private/mainwindow.h>

 #include <memory>

 #if ! UDJAT_CHECK_VERSION(1,2,1)
	#include <udjat/tools/url.h>
	#ifndef _WIN32
		#include <grp.h>
		#include <sys/types.h>
		#include <pwd.h>
	#endif // _WIN32
 #endif // !UDJAT 1.2.1

 using namespace Udjat;
 using namespace std;

 MainWindow::Group::Group(const XML::Node &node) :
	Reinstall::Group{node},
	title{ (*this)["title"], Gtk::Align::START },
	sub_title{ XML::AttributeFactory(node,"sub-title").as_string(), Gtk::Align::START } {

	debug("Building group '",title.get_text().c_str(),"'");

	// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Grid.html

	set_hexpand(true);
	set_halign(Gtk::Align::FILL);

	set_vexpand(false);
	set_valign(Gtk::Align::START);

	contents.set_hexpand(true);
	contents.set_halign(Gtk::Align::FILL);

	contents.set_vexpand(false);
	contents.set_valign(Gtk::Align::START);
	contents.get_style_context()->add_class("item-box");

	get_style_context()->add_class("group-box");

	title.get_style_context()->add_class("group-title");
	sub_title.get_style_context()->add_class("group-subtitle");

	int margin = 0;

	auto icon = XML::AttributeFactory(node,"icon");
	if(icon) {
		debug("Using icon '",icon.as_string(),"'");
		margin = 1;

		Gtk::Image image;
		//image.set_icon_size(Gtk::IconSize::LARGE);
		image.set_pixel_size(32);
		image.get_style_context()->add_class("group-icon");
		image.set_from_icon_name(icon.as_string("image-missing"));

		attach(image,0,0,1,2);
		contents.get_style_context()->add_class("item-box-no-icon");

	} else {

		contents.set_margin_start(35);
		contents.get_style_context()->add_class("item-box-icon");

	}

	attach(title,margin,0);
	attach(sub_title,margin,1);
	attach(contents,margin,2);

	set_visible();

 }

 #if ! UDJAT_CHECK_VERSION(1,2,1)
 bool Udjat::XML::test(const XML::Node &node, const char *attrname, bool defvalue) {

	XML::Attribute attr{AttributeFactory(node,attrname)};
	if(!attr) {
		return defvalue;
	}

	const char *str = attr.as_string("");
	if(str && *str && strstr(str,"://")) {
		// It's an URL, test it.
		return URL{str}.test();
	}

#ifdef _WIN32
	if(!strcasecmp(str,"only-on-windows")) {
		return true;
	}
	if(!strcasecmp(str,"only-on-linux")) {
		return false;
	}
#else
	if(!strcasecmp(str,"only-on-windows")) {
		return false;
	}
	if(!strcasecmp(str,"only-on-linux")) {
		return true;
	}

	if(!strncasecmp(str,"groups:",6)) {

		auto allowed=String{ (const char *) (str+6) }.split(",");

		struct passwd *pw = getpwuid(getuid());
		if(!pw) {
			Logger::String{"Cant get current user groups"}.warning(PACKAGE_NAME);
			return false;
		}

		int ngroups = 0;
		getgrouplist(pw->pw_name, pw->pw_gid, NULL, &ngroups);
		if(!ngroups) {
			Logger::String{"User group list is empty"}.warning(PACKAGE_NAME);
			return false;
		}

		gid_t groups[ngroups];
		getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);

		for (int i = 0; i < ngroups; i++){
			struct group* gr = getgrgid(groups[i]);
			if(gr == NULL){
				Logger::String{"getgrgid error: ",strerror(errno)}.warning(PACKAGE_NAME);
				continue;
			}
			for(const auto &name : allowed) {
				if(!strcasecmp(name.c_str(),gr->gr_name)) {
					return true;
				}
			}
		}

		return false;
	}

#endif // _WIN32

	return attr.as_bool(defvalue);
 }
#endif

 void MainWindow::Group::push_back(const Udjat::XML::Node &node, std::shared_ptr<Udjat::Abstract::Object> child) {

	// Só insere o ítem se 'visible' for true.
	if(!XML::test(node,"visible",true)) {
		Logger::String("Ignoring '",node.attribute("name").as_string(),"' by test result").trace(name());
		return;
	}

	auto item = make_shared<MainWindow::Item>(node,child);

	MainWindow::getInstance().itens.push_back(item);
	contents.append(*item);
	item->set_visible();

	Reinstall::Group::push_back(node,child);
 }
