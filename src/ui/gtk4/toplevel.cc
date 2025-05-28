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
 #include <udjat/ui/progress.h>
 #include <udjat/ui/status.h>

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

 TopLevel::TopLevel() : Gtk::ApplicationWindow(), apply{"suggested-action",_("C_ontinue")}, cancel{"cancel-action",_("_Cancel")} {
 
	// Get rid of the gtk warnings.
	freopen("/dev/null","w",stderr);

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

	// Initialize, parse XML files.
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
 TopLevel::Label::Label(const char *style, const char *text, Gtk::Align align) : Gtk::Label{text} {
	get_style_context()->add_class(style);
	set_wrap(true);
	set_halign(align);
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
		
		this->Gtk::ApplicationWindow::close();

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
		std::shared_ptr<Item> active_item;					///< @brief The selected.

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

			Glib::signal_idle().connect_once([this,&node,action,&semaphore](){

				auto item = make_shared<Item>(node, action);
				items.push_back(item);

				// Process activation.
				item->signal_toggled().connect([this,item,action]() {

					auto context = item->get_style_context();

					if(item->get_active()) {
						if(active_item) {
							active_item->set_active(false);
						}
						active_item = item;
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

				if(XML::AttributeFactory(node,"default").as_bool()) {
					item->set_active(true);
				}

				item->set_visible(true);
				this->set_visible(true);

				sem_post(&semaphore);

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

  void TopLevel::activate() noexcept{

	if(action->confirmation && !action->confirmation->ask()) {
		Logger::String{"Action was cancelled"}.info(action->name());
		return;
	}

	/// @brief The GTK4 progress dialog.
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
		Progress() {

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

		~Progress() override {
			debug("Destroying GTK4 progress dialog");
			timer->destroy();
		}

		Udjat::Dialog::Progress & step(const unsigned int current = 0, const unsigned int total = 0) noexcept override {
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

		Udjat::Dialog::Progress & set(uint64_t current, uint64_t total, bool) noexcept override {
			this->current = current;
			this->total = total;
			changed = true;
			return *this;
		}

		Udjat::Dialog::Progress & url(const char *url) noexcept override {
			string u{url};
			Glib::signal_idle().connect_once([this,u](){
				idle = 1000;
				changed = false;
				bar.set_text(u);
			});
			return *this;
		}

	};


	/// @brief The GTK4 status dialog.
	class Status : public Gtk::Dialog, private Udjat::Dialog::Progress::Factory, private Udjat::Dialog::Status {
	private:
		Label main{"dialog-title",""}, subtitle{"dialog-subtitle",""};
		Gtk::Image side_icon;

		struct {
			string label;
			uint8_t changed = 0;
			bool valid = false;
			float fraction;
		} values;

		std::shared_ptr<Progress> progress;

	public:
		Status() : progress{std::make_shared<Progress>()} {

			debug("Building GTK4 status dialog");

			gtk_window_set_transient_for(
				GTK_WINDOW(gobj()),
				gtk_application_get_active_window(GTK_APPLICATION(g_application_get_default()))
			);

			set_modal(true);
			set_deletable(false);
			set_resizable(false);

			set_decorated(false);
			set_default_size(500,-1);

			get_style_context()->add_class("dialog-progress");

			subtitle.set_valign(Gtk::Align::START);

			Gtk::Grid view;
			view.set_valign(Gtk::Align::CENTER);
			view.set_hexpand(true);
			view.set_vexpand(false);
			view.set_column_spacing(3);
			view.set_row_spacing(3);
			view.set_row_homogeneous(false);
			view.set_column_homogeneous(false);
			view.set_margin(12);
			view.get_style_context()->add_class("dialog-contents");

			side_icon.get_style_context()->add_class("dialog-icon");
			side_icon.set_hexpand(false);
			side_icon.set_vexpand(false);
			// icon.set_icon_size(::Gtk::IconSize::LARGE);
			side_icon.set_pixel_size(45);
			side_icon.set_from_icon_name("logo");

			view.attach(main,1,0,1,1);
			view.attach(subtitle,1,1,1,1);
			view.attach(side_icon,0,0,1,2);
			view.attach(*progress,0,2,2,1);

#ifdef DEBUG
			progress->url("The url");
			main.set_text("The message");
			subtitle.set_text("The body");
#endif // DEBUG

			set_child(view);
			view.show();

			set_default_size(700, -1);

		}
		
		~Status() override {
			debug("Destroying GTK4 status dialog");
		}

		std::shared_ptr<Udjat::Dialog::Progress> ProgressFactory() const {
			return progress;
		}

		Udjat::Dialog::Status & title(const char *text) noexcept override {
			string str{text};
			Glib::signal_idle().connect_once([this,str](){
				main.set_text(str);
			});
			return *this;
		}

		Udjat::Dialog::Status & sub_title(const char *text) noexcept override {
			string str{text};
			progress->url("");
			Glib::signal_idle().connect_once([this,str](){
				subtitle.set_text(str);
			});
			return *this;
		}

		Udjat::Dialog::Status & icon(const char *icon_name) noexcept override {
			if(icon_name && *icon_name) {
				string str{icon_name};
				Glib::signal_idle().connect_once([this,str](){
					side_icon.set_from_icon_name(str);
				});
			}	
			return *this;
		}

		Udjat::Dialog::Status & step(unsigned int current, unsigned int total) noexcept override {
			progress->step(current,total);
			return *this;
		}
	
		Udjat::Dialog::Status & show() noexcept override{
			Gtk::Dialog::set_visible(true);
			return *this;
		}

		Udjat::Dialog::Status & hide() noexcept override {
			Gtk::Dialog::set_visible(false);
			return *this;
		}

		Udjat::Dialog::Status & busy(bool enable) noexcept {
			if(enable) {
				progress->url(_("Please wait..."));
			}
			return *this;
		}

		Udjat::Dialog::Status & busy(const char *text) noexcept {
			progress->url(text);
			return *this;
		}

	};

	// Activate the action.
	auto status = new Status();
	status->present();

	ThreadPool::getInstance().push([this,status](){
		
		Reinstall::Application::activate();

		// Close progress popup.
		Glib::signal_idle().connect_once([status](){
			delete status;
		});

	});

  };
