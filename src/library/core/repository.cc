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
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/tools/xml.h>
 #include <libreinstall/repository.h>

 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	Repository::Repository(const Udjat::XML::Node &node)
#if UDJAT_CHECK_VERSION(1,2,0)
		:	name{XML::QuarkFactory(node,"name")}, slpclient{node},
			remote{XML::QuarkFactory(node,"remote")},
			local{XML::QuarkFactory(node,"local")},
			kparm{node} {
#else
		:	name{XML::QuarkFactory(node,"name").c_str()}, slpclient{node},
			remote{XML::QuarkFactory(node,"remote").c_str()},
			local{XML::QuarkFactory(node,"local").c_str()},
			kparm{node} {
#endif // UDJAT_CHECK_VERSION

		if(!(name && *name)) {
			throw runtime_error("Required attribute 'name' is missing or invalid");
		}

#if UDJAT_CHECK_VERSION(1,2,0)
		if(!(remote && *remote)) {
			remote = XML::QuarkFactory(node,"url");
		}
#else
		if(!(remote && *remote)) {
			remote = XML::QuarkFactory(node,"url").c_str();
		}
#endif // UDJAT_CHECK_VERSION

		if(!(remote && *remote)) {
			throw runtime_error("Required attribute 'remote' is missing or invalid");
		}

	}

	Udjat::URL Repository::path(const char *filepath) const {

		if(local && *local) {
			URL url{"file://",local};
			url += filepath;
			if(url.test() == 200) {
				Logger::String{"Accepting local '",url.c_str(),"'"}.trace(name);
				return url;
			}
			Logger::String{"Rejecting local '",url.c_str(),"'"}.warning(name);
		}

		// Return empty url
		return Udjat::URL{};
	}

	static Udjat::URL sanitize(const char *url) {
		return Udjat::URL{url};
	}

	Udjat::URL Repository::url() const {

		Repository *repo = const_cast<Repository *>(this);

		if(slpclient) {

			// SLP is set, try it.
			string url = slpclient.url();
			if(!url.empty()) {
				Logger::String{"Using slp URL ",url.c_str()}.trace(name);
				if(repo) {
					repo->kparm.val = kparm.slpval;
				}
				return sanitize(url.c_str());
			}

		}

		Logger::String{"Using default URL ",remote}.trace(name);
		if(repo) {
			repo->kparm.val = remote;
		}

		return sanitize(remote);
	}

	void Repository::set_kernel_parameter(const Udjat::XML::Node &node) {

		if(kparm) {
			Logger::String{"Kernel parameter was already set to '",kparm.name()}.warning(name);
		}

#if UDJAT_CHECK_VERSION(1,2,0)
		kparm.set_name(XML::QuarkFactory(node,"name"));
#else
		kparm.set_name(XML::QuarkFactory(node,"name").c_str());
#endif // UDJAT_CHECK_VERSION

	}

 }
