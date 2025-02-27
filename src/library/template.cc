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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/file/text.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>

 #include <unistd.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	Template::Template(const Udjat::XML::Node &node)
		: Udjat::NamedObject{node} {

		// Get marker.
		{
			const char *sMarker = node.attribute("marker").as_string(((std::string) Config::Value<String>("template","marker","$")).c_str());

			if(strlen(sMarker) > 1 || !sMarker[0]) {
				throw runtime_error("Marker attribute is invalid");
			}

			this->marker = sMarker[0];
		}

		// Get Type
		if(XML::AttributeFactory(node,"binary").as_bool(false)) {
			type = (Type) (type|Template::Binary);
		} else {
			type = (Type) (type|Template::Text);
		}

		if(XML::AttributeFactory(node,"script").as_bool()) {
			type = (Type) (type|Template::Script);
		}

		if(XML::AttributeFactory(node,"executable").as_bool()) {
			type = (Type) (type|Template::Script);
		}

		// Get URL
		{
			String str{node,"url",""};
			if(str.empty()) {
				throw runtime_error("Required attribute 'url' is missing or invalid");
			}

			str.unescape();
			str.expand(*this);

			url = str.as_quark();

			debug("Template URL set to '",url,"'");

		}

		// Get path
		{
			String str{node,"path",""};
			if(!str.empty()) {

				str.unescape();
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
			value = getenv("PWD");
			value += "/templates";
#else
			value = Application::DataDir{"templates"};
#endif // DEBUG
			return true;
		}

		return Udjat::NamedObject::getProperty(key,value);
	}

	void Template::load(const Udjat::Abstract::Object &parent, const Udjat::XML::Node &node, std::vector<std::shared_ptr<Template>> &templates) {

		parent.for_each(node, "template", [&templates](const XML::Node &child){
			templates.push_back(make_shared<Template>(child));
			return false;
		});

		debug("Got ",templates.size()," templates");

	}

	std::string Template::save(Udjat::Dialog::Progress &) {

		Udjat::URL url{this->url};

		if(url.local()) {
			debug("url=",url.c_str());
			return url.path();
		}

		throw runtime_error("Unable to handle remote template");

	}

	void Template::save(const Udjat::Abstract::Object &parent, Udjat::Dialog::Progress &progress) {

		Udjat::URL url{this->url};
		url.expand(parent);
		url.expand(*this);

		String filename{this->path};
		filename.expand(parent);
		filename.expand(*this);

		debug("Source URL: ",url.c_str());

		if(type & Type::Binary) {

			throw system_error(ENOTSUP, system_category(), _("Cant handle binary template"));

		} else {

			String text = url.get();
			text.expand(marker,parent);
			text.expand(marker,*this);
			debug("marker='",string{marker}.c_str(),"'");

			Udjat::File::Path::save(filename.c_str(),text.c_str());
		}




	}

	void Template::save(Udjat::Dialog::Progress &progress, const Udjat::Abstract::Object &object, const char *path) {

		auto from = save(progress);
		if( (type & Template::Text) != 0) {

			// Parse text file
			String contents{File::Text{from.c_str()}.c_str()};
			contents.expand(marker,object,true,true);

			// and save parsed contents.
			File::Text{path}.set(contents.c_str()).save();

		} else {

			throw runtime_error("Cant handle binary templates");

		}

		if((type & Template::Script) != 0) {
			if(chmod(path,0755) < 0) {
					throw system_error(errno,system_category(),_("Cant update template permissions"));
			}
		}

		debug("Template '",name(),"' saved on file ",path);

	}

 }

