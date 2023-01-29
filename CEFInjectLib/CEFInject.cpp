/*
Copyright 2021-2023 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "CEFInject.h"


#include <easywsclient.hpp>

#define _SSIZE_T_DEFINED
#include <easywsclient.cpp> // seems like a hack to me, but eh

#include <locale>
#include <codecvt>

namespace nlohmann {
	template <>
	struct adl_serializer<std::wstring> {
		static void to_json(json& j, const std::wstring& str) {
			j = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(str);
		}

		static void from_json(const json& j, std::wstring& str) {
			str = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(j.get<std::string>());
		}
	};
}

namespace CEFInject
{
	namespace internal
	{
		httplib::Client GetHttpClient(uint16_t port)
		{
			httplib::Client cli("localhost", port);
			cli.set_connection_timeout(1000);
			return cli;
		}

		static uint32_t msg_id = 0;

	}
	bool CEFDebugAvailable(uint16_t port)
	{
		auto cli = internal::GetHttpClient(port);
		cli.set_connection_timeout(500);
		cli.set_read_timeout(500);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				return true;
			}
		}
		return false;
	}

	std::vector<std::wstring> AvailableTabs(uint16_t port)
	{
		std::vector<std::wstring> tabs;
		auto cli = internal::GetHttpClient(port);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				const auto json = nlohmann::json::parse(res->body);
				for (const auto& j : json) {
					tabs.push_back(j["title"].get<std::wstring>());
				}
			}
		}
		return tabs;
	}


	std::string InjectJs(const std::wstring& tabname, const std::wstring& js, uint16_t port)
	{
		auto cli = internal::GetHttpClient(port);
		if (auto res = cli.Get("/json")) {
			if (res->status == 200) {
				const auto json = nlohmann::json::parse(res->body);
				for (const auto& tab : json) {
					if (tab["title"].get<std::wstring>().starts_with(tabname)) {


#ifdef _WIN32
						INT rc;
						WSADATA wsaData;

						rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
						if (rc) {
							printf("WSAStartup Failed.\n");
							return {};
						}
#endif

						std::shared_ptr<easywsclient::WebSocket> ws{
							easywsclient::WebSocket::from_url(tab["webSocketDebuggerUrl"].get<std::string>())
						};
						if (ws)
						{
							ws->send(
								nlohmann::json{
										{"id", internal::msg_id++},
										{"method", "Runtime.evaluate"},
										{"params", {
											{"userGesture", true},
											{"expression", js}
										}}
								}.dump());
							std::string res;
							while (ws->getReadyState() != easywsclient::WebSocket::CLOSED) {
								ws->poll();
								ws->dispatch([&ws, &res](const std::string& message) {
									res = message;
								});
								ws->close();
							}
#ifdef _WIN32
							WSACleanup();
#endif
							return res;
						}
#ifdef _WIN32
						WSACleanup();
#endif
						return {};
					}
				}
			}
		}
	}

}