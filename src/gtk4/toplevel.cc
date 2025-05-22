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
  * @brief Implements gtk4 toplevel.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <glib/gi18n.h>
 #include <private/toplevel.h>
 #include <semaphore.h>

 #ifdef LOG_DOMAIN
	#undef LOG_DOMAIN
 #endif
 #define LOG_DOMAIN "toplevel"
 #include <udjat/tools/logger.h>

 #include <udjat/tools/configuration.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/mainloop.h>
 #include <string>
 #include <reinstall/group.h>
 #include <reinstall/action.h>

 using namespace std;
 using namespace Udjat;

 class TopLevel::Item : public Gtk::ToggleButton {
 private:

	class Label : public Gtk::Label {
	public:
		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Label.html
		Label(const Udjat::XML::Node &node, const char *style, const char *attrname) 
		: Gtk::Label{XML::AttributeFactory(node,attrname).as_string(), Gtk::Align::START} {
			get_style_context()->add_class(style);
		}
	
	} label, body;

	Gtk::Grid grid;
	Gtk::LinkButton help_button;

 public:
	Item(const Udjat::XML::Node &node, std::shared_ptr<Reinstall::Action> action) 
		: label{node,"action-title","title"}, body{node,"action-subtitle","sub-title"} {

		set_hexpand(true);
		set_vexpand(false);
		set_valign(Gtk::Align::START);
		set_halign(Gtk::Align::FILL);

		get_style_context()->add_class("action-button");
		grid.get_style_context()->add_class("action-container");

		const char *icon_name = XML::AttributeFactory(node,"icon-name").as_string();			
		int margin = 0;
		if(icon_name && *icon_name) {

			debug("Using icon '",icon_name,"'");
			margin = 1;
	
			Gtk::Image image;
			image.set_icon_size(Gtk::IconSize::LARGE);
			image.get_style_context()->add_class("action-icon");
			image.set_from_icon_name(icon_name);
	
			grid.attach(image,0,0,1,2);
		}
	
		grid.attach(label,margin,0);
		grid.attach(body,margin,1);
	
		set_child(grid);
	
		get_style_context()->add_class("action-inactive");
		
		set_sensitive(false);

		// Check availability
		ThreadPool::getInstance().push([this,action](){

			try {
	
				if(action && !action->initialize()) {
					Logger::String{"Action initialization has returned 'false', keeping it disabled"}.error(action->name());
					return;
				}
	
			} catch(const std::exception &e) {
	
				Logger::String{e.what()}.error(action->name());
				return;
	
			} catch(...) {
	
				Logger::String{"Unexpected error while initializing action"}.error(action->name());
				return;
	
			}
	
			Logger::String{"Initialization complete, enabling item"}.info(action->name());
			Glib::signal_idle().connect([this](){
				set_sensitive(true);
				return 0;
			});
	
		});
	
	
	}
	
 };

 TopLevel::TopLevel() : Gtk::ApplicationWindow() {
 
#ifdef DEBUG 
	get_style_context()->add_class("devel");
#endif

	// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1ApplicationWindow.html
	{
		auto css = Gtk::CssProvider::create();
#ifdef DEBUG
		css->load_from_path("./stylesheet.css");
#else
		css->load_from_path(Application::DataFile("stylesheet.css").c_str());
#endif // DEBUG
		get_style_context()->add_provider_for_display(Gdk::Display::get_default(),css,GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}

	{
#ifdef DEBUG
		std::string iconpath{"./icons"};
#else
		Udjat::Application::DataDir iconpath{"icons"};
#endif // DEBUG

		Logger::String{"Searching '",iconpath.c_str(),"' for customized icons"}.trace();

		// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1IconTheme.html
		Gtk::IconTheme::get_for_display(Gdk::Display::get_default())->add_search_path(iconpath);

	}

	set_deletable(false);
	set_default_size(800, 600);

	// Set up layout
	set_title(Config::Value<string>("toplevel","title",_("System reinstallation")));
	set_child(hbox);
	hbox.append(sidebar);
	hbox.append(vbox);

	// Title
	{
		Gtk::Label title{_("Select an option")};
		title.get_style_context()->add_class("main-title");
		title.set_hexpand(true);
		title.set_vexpand(false);
		title.set_valign(Gtk::Align::START);
		vbox.append(title);
	}

	// Options
	{
		optionbox.set_margin_end(6);
		optionbox.set_hexpand(true);
		optionbox.set_vexpand(true);
		viewport.set_child(optionbox);
		viewport.set_hexpand(true);
		viewport.set_vexpand(true);
		vbox.append(viewport);
	}

	// Button bar
	{
		buttons.set_hexpand(false);
		buttons.set_vexpand(false);
		buttons.set_valign(Gtk::Align::END);
		buttons.set_halign(Gtk::Align::END);
		buttons.set_homogeneous();	
		buttons.get_style_context()->add_class("button-box");
		buttons.set_spacing(3);
		buttons.append(cancel);
		buttons.append(apply);
		set_default_widget(apply);
		apply.set_sensitive(false);
		vbox.append(buttons);

		cancel.signal_clicked().connect([&]() {
			Gtk::Window::close();
		});

		apply.signal_clicked().connect([&]() {
			activate();
		});
		
	}

	// TODO: Show loading popup.

	set_sensitive(false);
	ThreadPool::getInstance().push([this](){

		try {

			load_options();
			Glib::signal_idle().connect([this](){
				set_sensitive(true);
				present();
				return 0;
			});
	
		} catch(const std::exception &e) {
			failed(e);
		}

	});

 }

 TopLevel::~TopLevel() {
 
 }

 TopLevel::SideBar::SideBar() : Gtk::Box{Gtk::Orientation::VERTICAL} {
	get_style_context()->add_class("toplevel-sidebar");
	set_hexpand(false);
	set_vexpand(true);
	set_homogeneous(false);

#ifdef DEBUG
	Config::Value<string> path{"defaults","sidebar-logo","./icons/logo.svg"};
#else
	Config::Value<string> path{"defaults","sidebar-logo",Application::DataFile("icons/logo.svg").c_str()};
#endif // DEBUG

	try {

		Logger::String{"Getting logo from '",path.c_str(),"'"}.trace("sidebar");

		logo.set_pixel_size(128);
		logo.set(Gdk::Pixbuf::create_from_file(path.c_str()));
		logo.get_style_context()->add_class("toplevel-sidebar-logo");
		append(logo);

	} catch(const std::exception &e) {

		Logger::String{path.c_str(),": ",e.what()}.error("sidebar");

	}
	
 }

 // https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Label.html
 TopLevel::Label::Label(const char *style, const char *text) : Gtk::Label{text} {
	get_style_context()->add_class(style);
	set_wrap(true);
	set_halign(Gtk::Align::START);
	set_hexpand(true);
	set_vexpand(false);
 }

 TopLevel::Button::Button(const char *style, const char *text) : Gtk::Button{text} {
	// https://gnome.pages.gitlab.gnome.org/gtkmm/classGtk_1_1Button.html
	get_style_context()->add_class(style);
	set_halign(Gtk::Align::END);
	get_style_context()->add_class("pill");
	set_hexpand(false);
	set_vexpand(false);
	set_use_underline(true);
  }

  void TopLevel::failed(const std::exception &e) noexcept {

	Logger::String err{e.what()};
	
	err.error();

	Glib::signal_idle().connect([this,&err](){

		present();

		// TODO: Show error popup.
		debug("Showing error popup");
		
		this->close();

		return 0;
	});

  }
  
  std::shared_ptr<Reinstall::Group> TopLevel::group_factory(const Udjat::XML::Node &node) {
	
	class Group : public Gtk::Grid, public Reinstall::Group {
	private:
		TopLevel::Label title{"group-title",""};			///< @brief The group title.
		TopLevel::Label sub_title{"group-subtitle",""};		///< @brief The group sub-title.
		Gtk::Box contents{Gtk::Orientation::VERTICAL};		///< @brief The box with the options.
		std::vector<std::shared_ptr<Item>> items;			///< @brief The list of items in this group.

	public:
		Group(const Udjat::XML::Node &node) {

			get_style_context()->add_class("group-title-box");

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
				
		}

		void setup(const Udjat::XML::Node &node) override {
			title.set_text(XML::StringFactory(node,"title"));
			sub_title.set_text(XML::StringFactory(node,"sub-title"));
		}

		void push_back(const Udjat::XML::Node &node, std::shared_ptr<Reinstall::Action> action) override {

			sem_t semaphore;
			sem_init(&semaphore,0,0);

			Glib::signal_idle().connect([this,&node,action,&semaphore](){

				auto item = make_shared<Item>(node, action);
				items.push_back(item);

				// Process activation.
				item->signal_toggled().connect([item,action]() {

					auto context = item->get_style_context();

					if(item->get_active()) {
						context->remove_class("action-inactive");
						context->add_class("action-active");
						Application::getInstance().select(action);
					} else {
						context->remove_class("action-active");
						context->add_class("action-inactive");
					}

				});

				// Show item.
				contents.append(*item);
				item->set_visible(true);
				this->set_visible(true);

				sem_post(&semaphore);

				return 0;
			});

			sem_wait(&semaphore);
		}

	};
	
	// Build the group in the main thread, wait for it.
	Group *group = nullptr;

	sem_t semaphore;
	sem_init(&semaphore,0,0);

	Glib::signal_idle().connect([this,&node,&group,&semaphore](){
		group = new Group(node);
		group->set_visible(false);
		optionbox.append(*group);
		sem_post(&semaphore);
		return 0;
	});

	sem_wait(&semaphore);
	return std::shared_ptr<Group>{group};
  }

  void TopLevel::select(std::shared_ptr<Reinstall::Action> action) {

	apply.set_sensitive(action.get() != nullptr);
	Application::select(action);

  } 

  std::shared_ptr<Reinstall::Dialog> TopLevel::DialogFactory(const Udjat::XML::Node &node) {
	throw runtime_error{"DialogFactory not implemented"};
  } 

  void TopLevel::activate() noexcept{

	Reinstall::Application::activate();

  }
