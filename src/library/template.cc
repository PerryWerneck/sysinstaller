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
		: Udjat::NamedObject{node}, 
			escape{node.attribute("escape-control-characters").as_bool(false)},
			quirk{String{node,"quirk",""}.as_quark()} {

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
			Logger::String{node.path()," is using deprecated attribute 'script', use 'executable' instead"}.trace(name());
			mode = 0755;
		}

		if(XML::AttributeFactory(node,"executable").as_bool()) {
			mode = 0755;
		}

		// Get URL
		{
			String str{node,"url",""};
			if(str.empty()) {
				throw runtime_error(Logger::String{"Required attribute 'url' is missing or invalid on ",node.path()});
			}
			debug("Raw template URL is '",str.c_str(),"'");

			str.unescape();
			debug("Unescaped template URL is '",str.c_str(),"'");

			str.expand(*this,false);
			debug("Expanded template URL is '",str.c_str(),"'");

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

		debug("Getting template property '",key,"'");

		if(!strcasecmp(key,"template-dir")) {
#ifdef DEBUG
			value = getenv("PWD");
			value += "/templates";
#else
			value = Application::DataDir{"templates"};
#endif // DEBUG
			debug("Template directory set to '",value.c_str(),"'");
			return true;
		}

		if(!strcasecmp(key,"models-dir")) {
#ifdef DEBUG
			value = getenv("PWD");
			value += "/models";
#else
			value = Application::DataDir{"models"};
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

	void Template::save(const Udjat::Abstract::Object &parent, const char *path, const std::function<bool(uint64_t current, uint64_t total)> &progress) {

		String filename{path};
		filename.expand(parent);
		filename.expand(*this);

		Udjat::URL url{this->url};
		url.expand(parent);
		url.expand(*this);

		String text{url.get(progress)};
		text.expand(marker,parent);
		text.expand(marker,*this);

		// handle escape sequences.
		if(escape) {
			static const struct {
				const char *from;
				const char *to;
			} escapes[] = {
				{"&","\\&"},
			};

			debug("Text before escapes:\n",text.c_str());
			for(const auto &e : escapes) {
				for(auto pos = text.find(e.from); pos != std::string::npos; pos = text.find(e.from, pos + strlen(e.to))) {
					text.replace(pos, strlen(e.from), e.to);
				}
			}
			debug("Text after escapes:\n",text.c_str());
		}

		if(quirk && *quirk) {
			Config::Value<string> qvalue{"quirks",quirk};
			if(qvalue.empty()) {
				Logger::String{"Unknown quirk '",quirk,"' on template"}.warning(name());
			} else {
				auto values = String{qvalue.c_str()}.split(",");
				Logger::String{"Applying quirk ",quirk,": '",values[0].c_str(),"' -> '",values[1].c_str(),"'"}.trace(name());
				for(auto pos = text.find(values[0].c_str()); pos != std::string::npos; pos = text.find(values[0].c_str(), pos + values[1].size())) {
					text.replace(pos, values[0].size(), values[1].c_str());
				}
			}
		}

		if(script) {

			// Execute script, use stdout to set template result.
			throw runtime_error("Script templates are not supported yet");

		}

		// and save parsed contents.
		{
			File::Handler out{filename.c_str(),true};
			out.truncate();
			out.write(text.c_str(),text.size());
		}

		if(Logger::enabled(Logger::Debug)) {
			Logger::String(text.c_str()).write(Logger::Debug,name());
		}

		if(chmod(filename.c_str(),mode) < 0) {
			throw system_error(errno,system_category(),_("Cant update template permissions"));
		}

		debug("Template '",name(),"' saved on file ",path);

	}

 }

