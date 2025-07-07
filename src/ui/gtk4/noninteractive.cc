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
 #include <udjat/ui/progress.h>
 #include <reinstall/group.h>
 #include <reinstall/action.h>
 #include <memory>
 #include <semaphore.h>

 #ifdef LOG_DOMAIN
	#undef LOG_DOMAIN
 #endif
 #define LOG_DOMAIN "toplevel"
 #include <udjat/tools/logger.h>

 #pragma GCC diagnostic ignored "-Wattributes"
 #include <gtkmm.h>

 using namespace std;
 using namespace Udjat;

 NonInteractiveWindow::NonInteractiveWindow() : TopLevel() {
 
	// Setup the window
	set_default_size(700, -1);

	// Initialize, parse XML files.
	start();

 }

 NonInteractiveWindow::~NonInteractiveWindow() {
 
 }

 void NonInteractiveWindow::activate() noexcept {

 }

 std::shared_ptr<Udjat::Dialog::Progress> NonInteractiveWindow::ProgressFactory() const {
	return progress;
 }

 std::shared_ptr<Reinstall::Group> NonInteractiveWindow::group_factory(const Udjat::XML::Node &node) {

	class Group : public Reinstall::Group {
	private:
		NonInteractiveWindow &hwnd;

	public:

		Group(NonInteractiveWindow *h, const Udjat::XML::Node &node) : hwnd{*h} {
			// Initialize group with the XML node.
		}

		void parse(const Udjat::XML::Node &node) override {
			// Parse the XML node and build children.
			for (const auto &child : node.children()) {
				if(!Reinstall::Action::is_default(child)) {
					continue; // Skip non-action nodes.
				}
				push_back(node,make_shared<Reinstall::Action>(child));
			}
		}

		void push_back(const Udjat::XML::Node &node, shared_ptr<Reinstall::Action> action) override {

			sem_t semaphore;
			sem_init(&semaphore,0,0);

			Glib::signal_idle().connect_once([this,&node,action,&semaphore](){

				sem_post(&semaphore);
			});

			sem_wait(&semaphore);
		}

	};

	return std::make_shared<Group>(this, node);

 }


 