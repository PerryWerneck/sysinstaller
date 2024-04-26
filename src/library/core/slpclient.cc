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
 #include <udjat/tools/string.h>
 #include <libreinstall/slpclient.h>
 #include <udjat/ui/dialogs/progress.h>
 #include <list>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/net/ip/address.h>
 #include <netdb.h>

 #ifdef HAVE_LIBSLP
	#include <slp.h>
 #endif // HAVE_LIBSLP

 using namespace std;
 using namespace Udjat;

 using Progress = Udjat::Dialog::Progress;

 namespace Reinstall {

	SlpClient::SlpClient(const XML::Node &node)
#if UDJAT_CHECK_VERSION(1,2,0)
		: service_type{XML::QuarkFactory(node,"slp-service-type")},
			scope{XML::QuarkFactory(node,"slp-scope")},
			filter{XML::QuarkFactory(node,"slp-filter")},
			kparm{XML::QuarkFactory(node,"slp-kernel-parameter")},
			message{XML::QuarkFactory(node,"slp-search-message")},
			allow_local{XML::AttributeFactory(node,"slp-allow-local").as_bool(false)} {
#else
		: service_type{XML::QuarkFactory(node,"slp-service-type").c_str()},
			scope{XML::QuarkFactory(node,"slp-scope").c_str()},
			filter{XML::QuarkFactory(node,"slp-filter").c_str()},
			kparm{XML::QuarkFactory(node,"slp-kernel-parameter").c_str()},
			message{XML::QuarkFactory(node,"slp-search-message").c_str()},
			allow_local{XML::StringFactory(node,"slp-allow-local").as_bool(false)} {
#endif
	}

#ifdef HAVE_LIBSLP

	struct SRV_URL_CB_INFO  {
		SLPError callbackerr;
		std::list<String> responses;
	};

	static SLPBoolean slpcallback( SLPHandle, const char* srvurl, unsigned short, SLPError errcode, void *cookie ) {

		// http://www.openslp.org/doc/html/ProgrammersGuide/SLPFindSrvs.html

		if(errcode == SLP_OK || errcode == SLP_LAST_CALL) {

			if(srvurl && *srvurl) {
				Logger::String{"Got response '",srvurl,"'"}.trace("slp");
				((SRV_URL_CB_INFO *) cookie)->responses.emplace_back(srvurl);
			}

			*(SLPError*)cookie = SLP_OK;

		} else {

			Logger::String{"Got error '",errcode,"' on query"}.error("slp");
			*(SLPError*)cookie = errcode;

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

	Udjat::URL SlpClient::url() const {

		Udjat::URL found;

		// Detect URL
		Progress &progress{Progress::instance()};

		std::string dialog_sub_title{progress.message()};

		progress.message((message && *message) ? message : _("Searching for service"));

		progress.url(service_type);
		progress.pulse();

		// https://github.com/ManageIQ/slp/blob/master/examples/raw_example.c
		// https://docs.oracle.com/cd/E19455-01/806-0628/6j9vie80v/index.html
		SLPError err;
		SLPHandle hSlp;

		err = SLPOpen(NULL, SLP_FALSE, &hSlp);
		if(err != SLP_OK) {
			Logger::String{"SLPOpen has failed"}.warning("slpclient");
			return "";
		}

		SRV_URL_CB_INFO cbinfo;

		Logger::String{"Searching for ",service_type}.info("slpclient");
		err = SLPFindSrvs(
					hSlp,
					service_type,
					scope,
					filter,
					slpcallback,
					&cbinfo
				);

		// Check prefixes
		size_t prefix_length = strlen(service_type);

		cbinfo.responses.remove_if([this,prefix_length](String &url){

			const char *str = url.c_str();
			if(strncmp(str,service_type,prefix_length)) {
				Logger::String{"Ignoring invalid response '",url,"'"}.warning("slp");
				return true;
			}

			str += prefix_length;
			if(*str != ':') {
				Logger::String{"Ignoring unexpected response '",url,"'"}.warning("slp");
				return true;
			}

			Logger::String{"Accepting valid response '",url,"'"}.trace("slp");

			return false;
		});

		if(cbinfo.responses.empty()) {

			Logger::String{"No valid SLP response for ",service_type}.warning("slpclient");

		} else {

			Logger::String{cbinfo.responses.size()," SLP response(s) for ",service_type}.info("slpclient");

			if(err != SLP_OK) {

				Logger::String{"SLPFindSrvs has failed with rc ",err}.warning("slpclient");

			} else if(!allow_local) {

				// Ignore local addresses.
				vector<IP::Addresses> addresses; ///< @brief IP Addresses.
				IP::for_each([&addresses](const IP::Addresses &addr){
					addresses.push_back(addr);
					return false;
				});

				for(auto url=cbinfo.responses.begin(); url != cbinfo.responses.end() && found.empty(); url++) {

					SLPSrvURL *parsedurl = NULL;
					if(SLPParseSrvURL(url->c_str(),&parsedurl) != SLP_OK) {

						Logger::String{"Cant parse ",url->c_str()}.error("slpclient");

					} else if(parsedurl->s_pcHost && *parsedurl->s_pcHost) {

						struct addrinfo *ai;
						struct addrinfo hints;

						memset(&hints,0,sizeof(hints));
						hints.ai_family   = AF_UNSPEC;
						hints.ai_socktype = SOCK_STREAM;
						hints.ai_protocol = 0;
						hints.ai_flags    = AI_NUMERICSERV;

						string port;
						if(parsedurl->s_iPort) {
							port = std::to_string(parsedurl->s_iPort);
						}

						if(getaddrinfo(parsedurl->s_pcHost, port.c_str(), &hints, &ai)) {

							Logger::String{"Error resolving ",parsedurl->s_pcHost}.error("slpclient");

						} else {

							for(auto rp = ai;rp != NULL && found.empty(); rp = rp->ai_next) {

								sockaddr_storage addr{IP::Factory(rp->ai_addr)};
								bool remote = true;

								for(IP::Addresses &local : addresses) {
									if(local.address == addr) {
										remote = false;
										Logger::String {"Ignoring response ",std::to_string(addr)}.trace("slpclient");
									}
								}

								if(remote) {
									extract_url_from_response(*url, found);
								}
							}

							freeaddrinfo(ai);
						}

						SLPFree(parsedurl);

					} else {

						Logger::String{"Ignoring response '",*url,"'"}.info("slp");

					}

				}


			} else {

				// Get first address.
				for(String &response : cbinfo.responses) {
					SLPSrvURL *parsedurl = NULL;
					if(SLPParseSrvURL(response.c_str(),&parsedurl) != SLP_OK) {

						Logger::String{"Cant parse ",response.c_str()}.error("slp");

					} else {

						extract_url_from_response(response, found);

						SLPFree(parsedurl);

						if(!found.empty()) {
							break;
						}
					}

				}
			}
		}

		SLPClose(hSlp);

		progress.message(dialog_sub_title.c_str());
		progress.url(found.c_str());
		progress.pulse();

		return found;

	}

 #else

	Udjat::URL SlpClient::url() const {
		throw runtime_error(_("Unable to use SLP protocol because the core library was built without it"));
	}

 #endif // HAVE_LIBSLP

 }

