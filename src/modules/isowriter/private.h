/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>

 #include <memory>

 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/file/handler.h>

 #include <udjat/ui/menu.h>
 #include <udjat/ui/dialog.h>
 #include <udjat/ui/dialogs/progress.h>

 #include <libreinstall/popup.h>

 /// @brief Simple image writer, just write an ISO to USB Storage.
 class UDJAT_PRIVATE MenuItem : public Udjat::NamedObject, public Udjat::Menu::Item {
 protected:

	/// @brief Action dialogs.
	struct Dialogs {

		/// @brief The action title.
		const char *title = "";

		/// @brief Confirmation dialog (Yes/No/Quit)
		Udjat::Dialog confirmation;

		/// @brief Progress dialog.
		Udjat::Dialog progress;

		/// @brief Success dialog.
		Popup success;

		/// @brief Failed dialog.
		Popup failed;

#if UDJAT_CHECK_VERSION(1,2,0)
		Dialogs(const Udjat::XML::Node &node) :
			title{Udjat::XML::QuarkFactory(node,"title")},
			confirmation{"confirmation",node},
			progress{"progress",node},
			success{"success",node},
			failed{"failed",node} {
		}
#else
		Dialogs(const Udjat::XML::Node &node) :
			title{Udjat::XML::QuarkFactory(node,"title").c_str()},
			confirmation{"confirmation",node},
			progress{"progress",node},
			success{"success",node},
			failed{"failed",node} {
		}
#endif // UDJAT_CHECK_VERSION

	} dialogs;

 protected:

	/// @brief Get ISO file.
	virtual std::shared_ptr<Udjat::File::Handler> get_file(Udjat::Dialog::Progress &dialog) = 0;

 public:

	MenuItem(const Udjat::XML::Node &node) : Udjat::NamedObject{node}, Udjat::Menu::Item{node}, dialogs{node} {
	}

	/// @brief Activate menu option.
	void activate() override;

 };

 /// @brief Simple image writer, just write an ISO to USB Storage.
 class UDJAT_PRIVATE IsoWriter : public MenuItem {
 private:
	const char *remote;
	const char *local;

 protected:

	std::shared_ptr<Udjat::File::Handler> get_file(Udjat::Dialog::Progress &dialog) override;

 public:

#if UDJAT_CHECK_VERSION(1,2,0)
	IsoWriter(const Udjat::XML::Node &node)
		: MenuItem(node), remote{Udjat::XML::QuarkFactory(node,"remote")}, local{Udjat::XML::QuarkFactory(node,"local")} {
	}
#else
	IsoWriter(const Udjat::XML::Node &node)
		: MenuItem(node), remote{Udjat::XML::QuarkFactory(node,"remote").c_str()}, local{Udjat::XML::QuarkFactory(node,"local").c_str()} {
	}
#endif // UDJAT_CHECK_VERSION

 };


 /// @brief Format and write files.
 class UDJAT_PRIVATE DiskWriter : public MenuItem {
 protected:

	std::shared_ptr<Udjat::File::Handler> get_file(Udjat::Dialog::Progress &dialog) override;

 public:

	DiskWriter(const Udjat::XML::Node &node) : MenuItem(node) {
	}

 };

  /*
 #include <reinstall/action.h>

 /// @brief Simple image writer, just write an ISO to USB Storage.
 class UDJAT_PRIVATE IsoWriter : public Reinstall::Action {
 public:
	IsoWriter(const pugi::xml_node &node);
	std::shared_ptr<Reinstall::Builder> BuilderFactory() override;

 };

 /// @brief Format and write files.
 class UDJAT_PRIVATE DiskWriter : public Reinstall::Action {
 private:
	const char *fsname;

 public:
	DiskWriter(const pugi::xml_node &node);
	std::shared_ptr<Reinstall::Builder> BuilderFactory() override;
	void post(std::shared_ptr<Reinstall::Writer> writer) override;

 };
 */
