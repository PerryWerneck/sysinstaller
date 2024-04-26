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

 #include <config.h>
 #include <libreinstall/source.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/intl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <fcntl.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	/// @brief Build a file source.
	/// @param node Definitions for file source.
	/// @param defpath if true use default image path if attribute 'path' is empty.
	Source::Source(const Udjat::XML::Node &node, bool defpath) :
#if UDJAT_CHECK_VERSION(1,2,0)
		Udjat::NamedObject{ node },
		path{ node },
		reponame{ XML::QuarkFactory(node,"repository") },
		slpclient{ node },
		imgpath{ XML::QuarkFactory(node,"path") } {
#else
		Udjat::NamedObject{ node },
		path{ node },
		reponame{ XML::QuarkFactory(node,"repository").c_str() },
		slpclient{ node },
		imgpath{ XML::QuarkFactory(node,"path").c_str() } {
#endif
		if(defpath && !(imgpath && *imgpath)) {

			// No imgpath, use standard.

			if(path.remote[0] == '/') {

				// The remote path is relative, use it as image path.
				imgpath = path.remote;

			} else {

				// No image path, how can I store this source?
				const char *ptr = strrchr(path.local,'/');
				if(!ptr) {
					ptr = path.local;
				}

				imgpath = String{ptr}.as_quark();
				Logger::String{"Cant determine image path, using default '",imgpath,"'"}.warning(name());
			}

		}

	}

	Udjat::URL Source::local() const {

		if(path.local && *path.local && access(path.local,R_OK) == 0) {
			return (Udjat::URL{"file://"} + path.local);
		}

		return Udjat::URL{};
	}

	Udjat::URL Source::remote() const {

		if(slpclient) {

			// SLP is enabled, search it.
			URL url = slpclient.url();
			if(!url.empty()) {
				return url;
			}

		}

		if(path.remote && *path.remote) {
			return URL{path.remote};
		}

		throw runtime_error(Logger::Message{_("Can't determine source URL for '{}'"),name()});

	}

	String Source::image_path() const {
		return String{imgpath};
	}

	void Source::prepare(Source::Files &files) {
		prepare(this->local(),this->remote(),files);
	}

 }
