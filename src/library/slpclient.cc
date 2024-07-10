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
  * @brief Implements internal SLP client.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/progress.h>
 #include <mutex>

 #ifdef HAVE_LIBSLP
	#include <slp.h>
 #endif // HAVE_LIBSLP

 #include <unistd.h>
 #include <list>

 #include <private/slpclient.h>

 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	SLPClient::SLPClient(const Udjat::XML::Node &node) {
		service_type = XML::QuarkFactory(node,"slp-service-type");
		scope_list = XML::QuarkFactory(node,"slp-scope-list");
		filter = XML::QuarkFactory(node,"slp-filter");
		allow_local = XML::AttributeFactory(node,"slp-allow-local").as_bool(false);
	}

	std::shared_ptr<SLPClient> SLPClient::Factory(const Udjat::XML::Node &node) {

		// TODO: Avoid duplicated objects.

		return std::make_shared<SLPClient>(node);
	}

 #ifdef HAVE_LIBSLP

	struct SRV_URL_CB_INFO  {
		SLPError callbackerr;
		list<String> responses;
	};

	static SLPBoolean slpcallback( SLPHandle, const char* srvurl, unsigned short lifetime, SLPError errcode, void *cookie ) {

		// http://www.openslp.org/doc/html/ProgrammersGuide/SLPFindSrvs.html

		if(errcode == SLP_OK || errcode == SLP_LAST_CALL) {

			if(srvurl && *srvurl) {
				Logger::String{"Got response '",srvurl,"'"}.trace("slp");
				((SRV_URL_CB_INFO *) cookie)->responses.emplace_back(srvurl);
			}

			((SRV_URL_CB_INFO *) cookie)->callbackerr = SLP_OK;

		} else {

			Logger::String{"Got error '",errcode,"' on query"}.error("slp");
			((SRV_URL_CB_INFO *) cookie)->callbackerr = errcode;

		}

		return SLP_TRUE;

	}

	static void extract_url_from_response(const std::string &from, std::string &to) {

		const char *ptr = from.c_str();
		for(size_t ix = 0; ix < 2; ix++) {
			ptr = strchr(ptr,':');
			if(!ptr) {
				Logger::String{"Rejecting bad formatted response '",from,"'"}.warning("slp");
			}
			ptr++;
		}

		to.assign(ptr);

	}
#endif // HAVE_LIBSLP

	const char * SLPClient::url() {

		static mutex guard;
		lock_guard<mutex> lock(guard);

#ifdef HAVE_LIBSLP
		if(service_type && *service_type && !query.complete) {

			Dialog::Progress &dialog = Dialog::Progress::getInstance();
			dialog.url(service_type);

			// https://github.com/ManageIQ/slp/blob/master/examples/raw_example.c
			// https://docs.oracle.com/cd/E19455-01/806-0628/6j9vie80v/index.html
			SLPError err;
			SLPHandle hSlp;

			err = SLPOpen(NULL, SLP_FALSE, &hSlp);
			if(err != SLP_OK) {
				Logger::String{"SLPOpen has failed"}.warning("slp");
				return "";
			}

			SRV_URL_CB_INFO cbinfo;

			Logger::String{"Searching for ",service_type}.info("slp");
			err = SLPFindSrvs(
						hSlp,
						service_type,
						scope_list,
						filter,
						slpcallback,
						&cbinfo
					);





			SLPClose(hSlp);
			query.complete = true;
		}
#endif // HAVE_LIBSLP

		return query.url.c_str();
	}

 }

