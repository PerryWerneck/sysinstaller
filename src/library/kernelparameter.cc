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
  * @brief Implements kernel parameters.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>
 #include <vector>
 #include <unordered_map>
 #include <udjat/tools/quark.h>

 #include <reinstall/tools/kernelparameter.h>
 #include <reinstall/tools/repository.h>

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	class UDJAT_PRIVATE KParm : public KernelParameter {
	private:
		const char *val;

	public:
		KParm(const char *name, const char *value) : KernelParameter{name}, val{value} {
		}

		KParm(const Udjat::XML::Node &node) : KernelParameter{node}, val{expand(node,"value")} {
		}

		std::string value(const Udjat::Abstract::Object &object) const override {
			return KernelParameter::value(object,val);
		}

	};


	std::vector<KernelParameter::Preset> KernelParameter::presets;

	KernelParameter::Preset::Preset(const char *n, const char *v)
		: name{Udjat::Quark{n}.c_str()}, value{Quark{v}.c_str()} {
	}

	KernelParameter::~KernelParameter() {
	}

	const char * KernelParameter::expand(const Udjat::XML::Node &node, const char *attrname) {
		return String{node,attrname}.expand(node).expand().as_quark();
	}

	std::string KernelParameter::value(const Udjat::Abstract::Object &object, const char *str) {
		String expanded{str};
		expanded.expand(object);
		return expanded;
	}

	/*
	std::string KernelParameter::value(const Udjat::Abstract::Object &object) const {
		return String{refvalue}.expand(object);
	}
	*/

	void KernelParameter::load(const Udjat::XML::Node &node, std::vector<std::shared_ptr<KernelParameter>> &kparms) {

		// Use map to avoid add of the same key more than one time.
		std::unordered_map<std::string, std::shared_ptr<KernelParameter>> keys;

		if(kparms.empty()) {

			for(const auto &value : presets) {

				if(keys.find(value.name) == keys.end()) {
					auto kparm = make_shared<KParm>(value.name,value.value);
					keys[value.name] = kparm;
					kparms.push_back(kparm);
				}

			}

		} else {

			for(auto kparm : kparms) {
				keys[kparm->parameter_name()] = kparm;
			}

		}

		for(auto parent = node;parent;parent = parent.parent()) {

			// First search for 'kernel-parameter' nodes.
			for(auto child = parent.child("kernel-parameter");child;child = child.next_sibling("kernel-parameter")) {
				auto kparm = make_shared<KParm>(child);
				if(keys.find(kparm->parameter_name()) == keys.end()) {
					keys[kparm->parameter_name()] = kparm;
					kparms.push_back(kparm);
				}
			}

			// Then search for repositories.
			for(auto child = parent.child("repository");child;child = child.next_sibling("repository")) {

				auto kparm = Repository::Factory(child);
				auto name = kparm->parameter_name();

				if(name && *name && keys.find(name) == keys.end()) {
					keys[name] = kparm;
					kparms.push_back(kparm);
				}

			}

			// Then search for driver-update-disks


		}

	}

	std::string KernelParameter::join(const Udjat::Abstract::Object &object, const std::vector<std::shared_ptr<KernelParameter>> &kparms) {

		std::string result;

		for(auto kparm : kparms) {
			if(!result.empty()) {
				result += " ";
			}
			result += kparm->parameter_name();
			result += "=";
			result += kparm->value(object);
		}

		return result;
	}

	void KernelParameter::preset(const char *arg) {
		const char *ptr = strchr(arg,'=');

		if(!ptr) {
			ptr = strchr(arg,':');
		}

		if(!ptr) {
			throw runtime_error("Invalid kernel parameter definition");
		}

		preset(string{arg,(size_t) (ptr-arg)}.c_str(),ptr+1);
	}

 }

