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

 using namespace std;
 using namespace Udjat;

 namespace Reinstall {

	std::vector<KernelParameter::Preset> KernelParameter::presets;

	KernelParameter::Preset::Preset(const char *n, const char *v)
		: name{Quark{n}.c_str()},value{Quark{v}.c_str()} {
	}

	KernelParameter::KernelParameter(const Udjat::XML::Node &node, const char *attrname)
		: Udjat::NamedObject{node}, refvalue{String{node,attrname}.expand(node).expand().as_quark()} {
	}

	KernelParameter::~KernelParameter() {
	}

	std::string KernelParameter::value(const Udjat::Abstract::Object &object) const {
		return String{refvalue}.expand(object);
	}

	void KernelParameter::load(const Udjat::XML::Node &node, std::vector<std::shared_ptr<KernelParameter>> &kparms) {

		// Use map to avoid add of the same key more than one time.
		std::unordered_map<std::string, std::shared_ptr<KernelParameter>> keys;

		if(kparms.empty()) {

			for(const auto &value : presets) {

				if(keys.find(value.name) == keys.end()) {
					auto kparm = make_shared<KernelParameter>(value.name);
					kparm->refvalue = value.value;
					keys[value.name] = kparm;
					kparms.push_back(kparm);
				}

			}

		} else {

			for(auto kparm : kparms) {
				keys[kparm->name()] = kparm;
			}

		}

		for(auto parent = node;parent;parent = parent.parent()) {

			// First search for 'kernel-parameter' nodes.
			for(auto child = parent.child("kernel-parameter");child;child = child.next_sibling("kernel-parameter")) {
				auto kparm = make_shared<KernelParameter>(child);
				if(keys.find(kparm->name()) == keys.end()) {
					keys[kparm->name()] = kparm;
					kparms.push_back(kparm);
				}
			}

			// Then search for driver-update-disks

			// Then search for repositories.

		}

	}

	std::string KernelParameter::join(const Udjat::Abstract::Object &object, const std::vector<std::shared_ptr<KernelParameter>> &kparms) {

		std::string result;

		for(auto kparm : kparms) {
			if(!result.empty()) {
				result += " ";
			}
			result += kparm->name();
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

