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

 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/script.h>
 #include <memory>

 #include <libreinstall/source.h>
 #include <libreinstall/builder.h>
 #include <libreinstall/repository.h>
 #include <libreinstall/kernelparameter.h>

 #include <udjat/ui/dialog.h>
 #include <udjat/ui/menu.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <udjat/tools/object.h>

 #include <libreinstall/popup.h>

 using Progress = Udjat::Dialog::Progress;

 namespace Reinstall {

	/// @brief Standard action.
	class UDJAT_API Action : public Udjat::NamedObject, public Udjat::Menu::Item {
	private:

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

		class UDJAT_API Script : public Udjat::Script {
		public:

			enum Type {
				Pre,
				Post
			};

			Script(const Udjat::XML::Node &node);

			inline bool operator==(Type type) const noexcept {
				return this->type == type;
			}

		private:

			Type type;

		};

		struct BootOptions {

			/// @brief The text for boot label.
			const char *label = "";

			/// @brief The boot theme for grub.
			const char *theme = "";

			BootOptions(const Udjat::XML::Node &node);

		} options;

		/// @brief Ask user for confirmation.
		virtual bool confirm() const;

		/// @brief List of repositories defined by XML.
		std::vector<Reinstall::Repository> repositories;

		/// @brief Get repository by name
		const Repository & repository(const char *name) const;

		/// @brief List of sources defined by XML.
		std::vector<std::shared_ptr<Reinstall::Source>> sources;

		/// @brief List of templates defined by XML.
		std::vector<Reinstall::Template> templates;

		/// @brief List of kernel parameters defined by XML.
		std::vector<std::shared_ptr<Reinstall::Kernel::Parameter>> kparms;

		/// @brief List of scripts
		std::vector<Script> scripts;

		/// @brief Get image builder.
		virtual std::shared_ptr<Reinstall::Builder> BuilderFactory() const;

		/// @brief Get image writer.
		virtual std::shared_ptr<Reinstall::Writer> WriterFactory() const;

		/// @brief Get files, apply templates (if required).
		virtual void prepare(Progress &progress, Source::Files &files) const;

		/// @brief Build iso image.
		virtual void build(Progress &progress, std::shared_ptr<Reinstall::Builder> builder, Source::Files &files) const;

		/// @brief Write iso image.
		virtual void write(Progress &progress, std::shared_ptr<Reinstall::Builder> builder, std::shared_ptr<Reinstall::Writer> writer) const;

		/// @brief The output defined by xml
		const struct OutPut {

			/// @brief The output filename.
			const char * filename = nullptr;

			/// @brief The required file length.
			unsigned long long length = 0;

			constexpr OutPut() = default;
			OutPut(const Udjat::XML::Node &node);

		} output;

		void reboot() noexcept;

	public:

		Action(Action *) = delete;
		Action(Action &) = delete;

		Action(const Udjat::XML::Node &node);
		~Action();

		std::string getProperty(const char *key) const;
		bool getProperty(const char *key, std::string &value) const override;

		void activate() override;

	};

 }
