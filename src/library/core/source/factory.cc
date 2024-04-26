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

 /**
  * @brief Implements source factory.
  */

 #include <config.h>
 #include <memory>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>

 #include <libreinstall/source.h>
 #include <libreinstall/sources/zip.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	std::shared_ptr<Source> Source::factory(const Udjat::XML::Node &node) {

#if UDJAT_CHECK_VERSION(1,2,0)
		switch(String{XML::StringFactory(node,"type","default")}.select("default","zip",nullptr)) {
#else
		switch(XML::StringFactory(node,"type","value","default").select("default","zip",nullptr)) {
#endif // UDJAT_CHECK_VERSION
		case 0: // Default
			return make_shared<Source>(node);

		case 1:	// zip
			return make_shared<ZipSource>(node);

		default:
			throw runtime_error("Unexpected repository type");
		}

	}

	std::shared_ptr<Source::File> Source::File::Factory(const char *path, const char *contents) {

		/// @brief Template text with resolved ${}.
		class Text : public File {
		private:
			const string text;

		public:
			Text(const char *path, const char *contents) : Source::File{path}, text{contents} {
			}

			virtual ~Text() {
			}

			bool remote() const noexcept override {
				return true;
			}

			const char * path() const override {
				throw runtime_error("Unexpected call to 'path' on text source");
			}

			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const override {
				writer(0,text.size(),text.c_str(),text.size());
			}

		};

		return make_shared<Text>(path,contents);

	}

 }
