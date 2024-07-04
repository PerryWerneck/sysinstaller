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
  * @brief Implements templates.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <reinstall/tools/template.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/quark.h>
 #include <stdexcept>

 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Template::Template(const Udjat::Abstract::Object &p, const Udjat::XML::Node &node, Type t)
		: Udjat::NamedObject{node}, parent{p}, type{t} {

		// Get URL
		{
			String str{node,"url",""};
			if(str.empty()) {
				throw runtime_error("Required attribute 'url' is missing or invalid");
			}

			str.unescape();
			str.expand(parent);
			str.expand(*this);

			url = str.as_quark();

			debug("Template URL set to '",url,"'");

		}

		// Get path
		{
			String str{node,"path",""};
			if(!str.empty()) {

				str.unescape();
				str.expand(parent);
				str.expand(*this);

				path = str.as_quark();

				debug("Template path set to '",url,"'");
			}


		}

	}

	Template::~Template() {

		if(!tempfilename.empty()) {
			unlink(tempfilename.c_str());
		}
	}

	bool Template::operator==(const char *path) const {

		const char *ptr = strrchr(path,'/');
		if(ptr && !strcmp(ptr+1,name())) {
			return true;
		}

		return false;
	}

	bool Template::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"template-dir")) {
#ifdef DEBUG
			value = "./templates";
#else
			value = Application::DataDir{"templates"};
#endif // DEBUG
			return true;
		}

		return Udjat::NamedObject::getProperty(key,value);
	}

	void Template::load(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node, std::vector<std::shared_ptr<Template>> &templates) {

		parent.for_each(node, "template", [&parent,&templates](const XML::Node &child){
			templates.push_back(make_shared<Template>(parent,child));
			return false;
		});

		debug("Got ",templates.size()," templates");

	}

 }

