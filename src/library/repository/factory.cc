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
  * @brief Implements Source repository.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/progress.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/configuration.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>
 #include <private/slpclient.h>
 #include <list>

 #ifdef HAVE_ZLIB
	#include <zlib.h>
 #endif // HAVE_ZLIB

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	std::vector<Repository::Preset> Repository::presets;

	Repository::Preset::Preset(const char *n, const char *v)
		: name{Udjat::Quark{n}.c_str()}, value{Quark{v}.c_str()} {
	}

	std::shared_ptr<Repository> Repository::Factory(const Udjat::XML::Node &node) {

		static mutex guard;
		lock_guard<mutex> lock(guard);

		const char * name = XML::StringFactory(node,"repository","install");

		static list<std::shared_ptr<Repository>> repositories;

		for(auto repository : repositories) {
			if(!strcasecmp(repository->name(),name)) {
				return repository;
			}
		}

		for(auto parent = node;parent;parent = parent.parent()) {

			for(auto child = parent.child("repository");child;child = child.next_sibling("repository")) {

				if(strcasecmp(child.attribute("name").as_string(),name)) {
					continue;
				}

				if(!strcasecmp(XML::StringFactory(child,"repository",""),name)) {
					Logger::String{"Ignoring circular dependency"}.warning("repository");
					continue;
				}

				auto repo = make_shared<Repository>(child);
				repositories.push_back(repo);

				for(const auto preset : presets) {

					if(!strcasecmp(repo->name(),preset.name)) {
						// Found preset.
						repo->url.remote = preset.value;
						Logger::String{"Using '",repo->url.remote,"' as remote URL"}.info(repo->name());
						repo->slpclient->clear();
					}

				}

				return repo;
			}

		}

		throw runtime_error(Logger::Message{_("Required repository '{}' not found"),name});

	}


 }


