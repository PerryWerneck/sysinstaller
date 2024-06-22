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
  * @brief Implements Source repository..
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/intl.h>

 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/repository.h>
 #include <private/slpclient.h>
 #include <list>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	std::shared_ptr<Repository> Repository::Factory(const Udjat::XML::Node &node) {

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

				return repo;
			}

		}

		throw runtime_error(Logger::Message{_("Required repository '{}' not found"),name});

	}

	Repository::Repository(const Udjat::XML::Node &node) : DataSource{node}, slpclient{SLPClient::Factory(node)} {

		if(url.local[0]) {
			Logger::String{"Using '",url.local,"' for local files"}.trace(name());
		}

		if(url.remote[0]) {
			Logger::String{"Using '",url.remote,"' for remote files"}.trace(name());
		}

	}

	Repository::~Repository() {
	}

 }

