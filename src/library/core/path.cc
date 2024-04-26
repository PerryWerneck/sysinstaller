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
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <udjat/tools/xml.h>
 #include <libreinstall/path.h>
 #include <udjat/tools/logger.h>

 using namespace Udjat;

 namespace Reinstall {

	Path::Path(const Udjat::XML::Node &node)
#if UDJAT_CHECK_VERSION(1,2,0)
		: Path{Udjat::XML::QuarkFactory(node,"remote"),Udjat::XML::QuarkFactory(node,"local")} {

		// Check legacy attribute 'url'.
		if(!(remote && *remote)) {
			remote = Udjat::XML::QuarkFactory(node,"url");
		}
#else
		: Path{Udjat::XML::QuarkFactory(node,"remote").c_str(),Udjat::XML::QuarkFactory(node,"local").c_str()} {

		// Check legacy attribute 'url'.
		if(!(remote && *remote)) {
			remote = Udjat::XML::QuarkFactory(node,"url").c_str();
		}
#endif // UDJAT_CHECK_VERSION

		// It there's no local path and remote is relative, use remote.
		if(!local[0] && (remote[0] == '/' || remote[0] == '.') && node.attribute("auto-detect-local-path").as_bool(true)) {
			// No local path and remote is relative, use remote as local
			local = remote;
		}

	}

 }

