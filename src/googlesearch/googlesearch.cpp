// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.

#include "googlesearch.h"

#include <iostream>
#include <fstream>

#include <curl/curl.h>
#include <imgui/imgui.h>

#include <stdio.h>


IMPLEMENT_PLUGIN(GoogleSearch)

// API documentation: https://developers.google.com/custom-search/v1/cse/list
static const std::string sApiKey("AIzaSyBHnMEOSkTM7Lazvou27FXgJb6M4hjn9uE");
static const std::string sSearchEngineId("016794710981670214596:sgntarey42m");

// This needs to be a static function as libcurl is a C library and will segfault if passed
// a local lambda.
static size_t WriteMemoryCallback(void* pContents, size_t size, size_t nmemb, void* pUserData)
{
	size_t realSize = size * nmemb;
	std::string& data = *reinterpret_cast<std::string*>(pUserData);
	size_t curDataSize = data.size();
	data.resize(curDataSize + realSize);
	memcpy(&data[curDataSize], pContents, realSize);
	return realSize;
}

GoogleSearch::GoogleSearch()
{
	//LoadRules();

	m_pCurlHandle = curl_easy_init();
}

GoogleSearch::~GoogleSearch()
{
	if (m_QueryThread.joinable())
	{
		m_QueryThread.join();
	}

	curl_easy_cleanup(m_pCurlHandle);
}

bool GoogleSearch::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;

	m_QueryThread = std::thread(GoogleSearch::ThreadMain, this);
	return true;
}

void GoogleSearch::OnMessageReceived(const nlohmann::json& message)
{

}

void GoogleSearch::DrawUI(ImGuiContext* pContext)
{
	ImGui::SetCurrentContext(pContext);

	if (ImGui::CollapsingHeader("Google search", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//ImGui::Text("Rules loaded: %d", m_Rules.size());
		//ImGui::Text("Queue size: %d", static_cast<int>(m_PendingResults));

		//if (ImGui::Button("View results"))
		//{
		//	m_ShowResultsUI = !m_ShowResultsUI;
		//}

		//if (m_ShowResultsUI)
		//{
		//	DrawResultsUI(&m_ShowResultsUI);
		//}
	}
}

void GoogleSearch::ThreadMain(GoogleSearch* pGoogleSearch)
{
	std::string rule("inurl%3A%22video.mjpg%22");

	std::stringstream url;
	url << "https://www.googleapis.com/customsearch/v1?q=" << rule << "&cx=" << sSearchEngineId << "&key=" << sApiKey;

	CURL* pCurlHandle = pGoogleSearch->m_pCurlHandle;
	char pErrorBuffer[CURL_ERROR_SIZE];
	curl_easy_setopt(pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer);
	curl_easy_setopt(pCurlHandle, CURLOPT_URL, url.str().c_str());
	curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, &pGoogleSearch->m_Data);
	curl_easy_setopt(pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(pCurlHandle, CURLOPT_TIMEOUT, 10L);

	if (curl_easy_perform(pCurlHandle) != CURLE_OK)
	{
		json message =
		{
			{ "type", "log" },
			{ "level", "error" },
			{ "plugin", "googlesearch" },
			{ "message", pErrorBuffer }
		};
		pGoogleSearch->m_pMessageCallback(message);
	}
	else
	{
		json data = json::parse(pGoogleSearch->m_Data);
		int a = 0;
		//pGeolocation->m_pMessageCallback(message);
	}
}

//void HTTPCameraDetector::LoadRules()
//{
//	std::ifstream file("plugins/httpcameradetector/rules.json");
//	if (file.is_open())
//	{
//		json jsonRules;
//		file >> jsonRules;
//
//		for (auto& jsonRule : jsonRules)
//		{
//			Rule cameraDetectionRule;
//			for (json::iterator it = jsonRule.begin(); it != jsonRule.end(); ++it)
//			{
//				const std::string& key = it.key();
//				if (key == "intitle")
//				{
//					if (it.value().is_array())
//					{
//						for (auto& text : it.value())
//						{
//							cameraDetectionRule.AddFilter(Rule::FilterType::InTitle, text);
//						}
//					}
//				}
//			}
//			m_Rules.push_back(cameraDetectionRule);
//		}
//	}
//}

//void HTTPCameraDetector::Scan(HTTPCameraDetector* pDetector, const std::string& url, const std::string& ipAddress, int port)
//{
//	CURL* pCurl = curl_easy_init();
//
//	char tagBuffer[32];
//	char attrBuffer[32];
//	char valBuffer[128];
//	char innerTextBuffer[1024];
//	HTMLSTREAMPARSER* hsp = html_parser_init();
//
//	html_parser_set_tag_to_lower(hsp, 1);
//	html_parser_set_attr_to_lower(hsp, 1);
//	html_parser_set_tag_buffer(hsp, tagBuffer, sizeof(tagBuffer));
//	html_parser_set_attr_buffer(hsp, attrBuffer, sizeof(attrBuffer));
//	html_parser_set_val_buffer(hsp, valBuffer, sizeof(valBuffer) - 1);
//	html_parser_set_inner_text_buffer(hsp, innerTextBuffer, sizeof(innerTextBuffer) - 1);
//
//	ScanCallbackData data;
//	data.pHsp = hsp;
//	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
//	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_callback);
//	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &data);
//	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
//	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10L);
//
//	curl_easy_perform(pCurl);
//
//	curl_easy_cleanup(pCurl);
//
//	json message =
//	{
//		{ "type", "http_server_scan_result" },
//		{ "url", url },
//		{ "ip_address", ipAddress },
//		{ "port", port },
//		{ "is_camera", EvaluateDetectionRules(pDetector, url, data.title) },
//		{ "title", data.title }
//	};
//	pDetector->m_pMessageCallback(message);
//}

//bool HTTPCameraDetector::EvaluateDetectionRules(HTTPCameraDetector* pDetector, const std::string& url, const std::string& title)
//{
//	for (auto& rule : pDetector->m_Rules)
//	{
//		if (rule.Match(url, title))
//		{
//			return true;
//		}
//	}
//	return false;
//}
