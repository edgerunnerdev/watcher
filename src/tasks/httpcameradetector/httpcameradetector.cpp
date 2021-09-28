//// This file is part of watcher.
////
//// watcher is free software: you can redistribute it and/or modify
//// it under the terms of the GNU General Public License as published by
//// the Free Software Foundation, either version 3 of the License, or
//// (at your option) any later version.
////
//// watcher is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU General Public License for more details.
////
//// You should have received a copy of the GNU General Public License
//// along with watcher. If not, see <https://www.gnu.org/licenses/>.
//
//#include "httpcameradetector.h"
//
//#include <iostream>
//#include <fstream>
//
//#include <curl/curl.h>
//#include <imgui/imgui.h>
//
//#include <stdio.h>
//
//#include "htmlstreamparser.h"
//#include "rule.h"
//
//IMPLEMENT_PLUGIN(HTTPCameraDetector)
//
//struct ScanCallbackData
//{
//	HTMLSTREAMPARSER* pHsp;
//	std::string title;
//};
//
//static size_t write_callback(void* buffer, size_t size, size_t nmemb, void* pData)
//{
//	ScanCallbackData* pCallbackData = reinterpret_cast<ScanCallbackData*>(pData);
//	HTMLSTREAMPARSER* pHsp = pCallbackData->pHsp;
//	size_t realsize = size * nmemb;
//	std::string titleEndTag = "/title";
//	size_t titleLen = 0;
//	for (size_t p = 0; p < realsize; p++)
//	{
//		char c = ((char*)buffer)[p];
//		if (c <= -1)
//		{
//			continue;
//		}
//
//		html_parser_char_parse(pHsp, ((char *)buffer)[p]);
//		if (html_parser_cmp_tag(pHsp, &titleEndTag[0], titleEndTag.size()))
//		{
//			titleLen = html_parser_inner_text_length(pHsp);
//			char* pTitle = html_parser_replace_spaces(html_parser_trim(html_parser_inner_text(pHsp), &titleLen), &titleLen);
//
//			if (titleLen > 0)
//			{
//				pTitle[titleLen] = '\0';
//				pCallbackData->title = pTitle;
//			}
//		}
//	}
//	return realsize;
//}
//
//HTTPCameraDetector::HTTPCameraDetector() :
//m_ThreadPool(8),
//m_PendingResults(0),
//m_ShowResultsUI(false)
//{
//	LoadRules();
//}
//
//HTTPCameraDetector::~HTTPCameraDetector()
//{
//
//}
//
//bool HTTPCameraDetector::Initialise(PluginMessageCallback pMessageCallback)
//{
//	m_pMessageCallback = pMessageCallback;
//	return true;
//}
//
//void HTTPCameraDetector::OnMessageReceived(const nlohmann::json& message)
//{
//	const std::string& messageType = message["type"];
//	if (messageType == "http_server_found")
//	{
//		m_PendingResults++;
//		ThreadPool::Job job = std::bind(HTTPCameraDetector::Scan, this, message["url"], message["ip_address"], message["port"]);
//		m_ThreadPool.Queue(job);
//	}
//	else if (messageType == "http_server_scan_result")
//	{
//		std::lock_guard<std::mutex> lock(m_ResultsMutex);
//		m_PendingResults--;
//		Result result;
//		result.url = message["url"];
//		result.title = message["title"];
//		result.isCamera = message["is_camera"];
//		m_Results.push_front(result);
//		if (m_Results.size() > 100)
//		{
//			m_Results.pop_back();
//		}
//	}
//}
//
//void HTTPCameraDetector::DrawUI(ImGuiContext* pContext)
//{
//	ImGui::SetCurrentContext(pContext);
//
//	if (ImGui::CollapsingHeader("HTTP camera detector", ImGuiTreeNodeFlags_DefaultOpen))
//	{
//		ImGui::Text("Rules loaded: %d", m_Rules.size());
//		ImGui::Text("Queue size: %d", static_cast<int>(m_PendingResults));
//
//		if (ImGui::Button("View results##HTTPCameraDetector"))
//		{
//			m_ShowResultsUI = !m_ShowResultsUI;
//		}
//
//		if (m_ShowResultsUI)
//		{
//			DrawResultsUI(&m_ShowResultsUI);
//		}
//	}
//}
//
//void HTTPCameraDetector::DrawResultsUI(bool* pShow)
//{
//	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
//	if (!ImGui::Begin("HTTP Camera Detector - results", pShow))
//	{
//		ImGui::End();
//		return;
//	}
//
//	std::lock_guard<std::mutex> lock(m_ResultsMutex);
//	ImGui::Columns(3);
//	ImGui::SetColumnWidth(0, 64);
//	ImGui::Separator();
//	ImGui::Text("Camera?"); ImGui::NextColumn();
//	ImGui::Text("URL"); ImGui::NextColumn();
//	ImGui::Text("Title"); ImGui::NextColumn();
//	ImGui::Separator();
//	for (Result& result : m_Results)
//	{
//		ImGui::Selectable(result.isCamera ? "Yes" : " ", true, ImGuiSelectableFlags_SpanAllColumns);
//		ImGui::NextColumn();
//		ImGui::Text(result.url.c_str());
//		ImGui::NextColumn();
//		ImGui::Text(result.title.c_str());
//		ImGui::NextColumn();
//	}
//	ImGui::Columns(1);
//	ImGui::End();
//}
//
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
//
//void HTTPCameraDetector::Scan(HTTPCameraDetector* pDetector, const std::string& url, const std::string& ipAddress, int port)
//{
//	if (url.rfind(".mjpg") != std::string::npos)
//	{
//		json message =
//		{
//			{ "type", "http_server_scan_result" },
//			{ "url", url },
//			{ "ip_address", ipAddress },
//			{ "port", port },
//			{ "is_camera", true },
//			{ "title", "" }
//		};
//		pDetector->m_pMessageCallback(message);
//	}
//	else
//	{
//		CURL* pCurl = curl_easy_init();
//
//		char tagBuffer[32];
//		char attrBuffer[32];
//		char valBuffer[128];
//		char innerTextBuffer[1024];
//		HTMLSTREAMPARSER* hsp = html_parser_init();
//
//		html_parser_set_tag_to_lower(hsp, 1);
//		html_parser_set_attr_to_lower(hsp, 1);
//		html_parser_set_tag_buffer(hsp, tagBuffer, sizeof(tagBuffer));
//		html_parser_set_attr_buffer(hsp, attrBuffer, sizeof(attrBuffer));
//		html_parser_set_val_buffer(hsp, valBuffer, sizeof(valBuffer) - 1);
//		html_parser_set_inner_text_buffer(hsp, innerTextBuffer, sizeof(innerTextBuffer) - 1);
//
//		ScanCallbackData data;
//		data.pHsp = hsp;
//		curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
//		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_callback);
//		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &data);
//		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
//		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10L);
//
//		curl_easy_perform(pCurl);
//
//		curl_easy_cleanup(pCurl);
//
//		json message =
//		{
//			{ "type", "http_server_scan_result" },
//			{ "url", url },
//			{ "ip_address", ipAddress },
//			{ "port", port },
//			{ "is_camera", EvaluateDetectionRules(pDetector, url, data.title) },
//			{ "title", data.title }
//		};
//		pDetector->m_pMessageCallback(message);
//	}
//}
//
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
