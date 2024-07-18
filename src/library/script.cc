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
  * @brief Implements script.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/tools/script.h>
 #include <reinstall/tools/repository.h>
 #include <stdexcept>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Script::Script(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node)
		: rtime{(Script::RunTime) String{XML::StringFactory(node,"type","post")}.select("pre","post",nullptr)} {

		Udjat::NamedObject::set(node);

		String text{node.child_value()};
		text.strip();

		if(text.empty()) {

			// Not text, get from URLs.

			URL attr{XML::StringFactory(node,"url")};

			url.remote = XML::QuarkFactory(node,"remote");
			url.local = XML::QuarkFactory(node,"local");

			bool local = (strncmp(attr.ComponentsFactory().scheme.c_str(),"file://",7) == 0);

			if(!url.local[0] && local) {
				url.local = attr.c_str();
				Logger::String{"Will get local script from",url.local}.trace(name());
			}

			if(!(url.remote[0] || local)) {
				url.remote = attr.c_str();
				Logger::String{"Will get remote script from",url.remote}.trace(name());
			}

		} else {

			// Has script code.
			Logger::String{"Using script from XML node"}.trace(name());

			const char marker = node.attribute("marker").as_string(((std::string) Config::Value<String>("string","marker","$")).c_str())[0];

			if(marker) {
				text.expand(marker,parent);
				text.expand(marker,node);
			}

			if(node.attribute("strip-lines").as_bool()) {
				String stripped;
				text.for_each("\n",[&stripped](const String &value) {
					stripped += const_cast<String &>(value).strip();
					stripped += "\n";
					return false;
				});
				this->code = stripped.as_quark();

			} else {
				this->code = text.as_quark();
			}

			if(Logger::enabled(Logger::Debug)) {
				Logger::String{"Parsed script:\n",this->code}.write(Logger::Debug,name());
			}

		}

		if(url.remote[0] == '.' || url.local[0] == '.' || url.remote[0] == '/' || url.local[0] == '/') {
			repository = Repository::Factory(node);
		}

	}

	void Script::load(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node, std::vector<std::shared_ptr<Script>> &scripts) {

		parent.for_each(node, "script", [&scripts, &parent](const XML::Node &child){
			scripts.push_back(make_shared<Script>(parent,child));
			return false;
		});

		debug("Got ",scripts.size()," scripts");

	}

 }


