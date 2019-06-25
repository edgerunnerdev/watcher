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

#include "httpcameradetector.h"

#include <iostream>
#include <fstream>

#include <curl/curl.h>
#include <imgui/imgui.h>

#include <stdio.h>

#include "htmlstreamparser.h"
#include "rule.h"

IMPLEMENT_PLUGIN(HTTPCameraDetector)

struct CameraScannerCallbackData
{
	HTMLSTREAMPARSER* pHsp;
	HTTPCameraDetector* pDetector;
};

static size_t write_callback(void* buffer, size_t size, size_t nmemb, void* pData)
{
	CameraScannerCallbackData* pCallbackData = reinterpret_cast<CameraScannerCallbackData*>(pData);
	HTMLSTREAMPARSER* pHsp = pCallbackData->pHsp;
	size_t realsize = size * nmemb;
	std::string titleEndTag = "/title";
	size_t titleLen = 0;
	for (size_t p = 0; p < realsize; p++)
	{
		char c = ((char*)buffer)[p];
		if (c <= -1)
		{
			continue;
		}

		html_parser_char_parse(pHsp, ((char *)buffer)[p]);
		if (html_parser_cmp_tag(pHsp, &titleEndTag[0], titleEndTag.size()))
		{
			titleLen = html_parser_inner_text_length(pHsp);
			char* pTitle = html_parser_replace_spaces(html_parser_trim(html_parser_inner_text(pHsp), &titleLen), &titleLen);

			if (titleLen > 0)
			{
				pTitle[titleLen] = '\0';
				//pCallbackData->pDetector->SetTitle(pTitle);
			}
		}
	}
	return realsize;
}

HTTPCameraDetector::HTTPCameraDetector() :
m_ThreadPool(8),
m_PendingResults(0)
{
	LoadRules();
}

HTTPCameraDetector::~HTTPCameraDetector()
{

}

bool HTTPCameraDetector::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

void HTTPCameraDetector::OnMessageReceived(const nlohmann::json& message)
{
	if (message["type"] == "http_server_found")
	{
		m_PendingResults++;
	}
}

void HTTPCameraDetector::DrawUI(ImGuiContext* pContext)
{
	ImGui::SetCurrentContext(pContext);

	if (ImGui::CollapsingHeader("HTTP camera detector", ImGuiTreeNodeFlags_DefaultOpen))
	{
		std::stringstream queueSizeSS;
		queueSizeSS << "Queue size: " << m_PendingResults;
		ImGui::Text(queueSizeSS.str().c_str());

		if (ImGui::TreeNode("Results (100 most recent)"))
		{
			std::lock_guard<std::mutex> lock(m_ResultsMutex);
			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, 32);
			for (Result& result : m_Results)
			{
				ImGui::Text(result.isCamera ? "x" : " ");
				ImGui::NextColumn();
				ImGui::Text(result.address.ToString().c_str());
				ImGui::NextColumn();
				ImGui::Text(result.title.c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::TreePop();
		}
	}
}

void HTTPCameraDetector::LoadRules()
{
	std::ifstream file("plugins/httpcameradetector/rules.json");
	if (file.is_open())
	{
		json jsonRules;
		file >> jsonRules;
		file.close();

		for (auto& jsonRule : jsonRules)
		{
			Rule cameraDetectionRule;
			for (json::iterator it = jsonRule.begin(); it != jsonRule.end(); ++it)
			{
				const std::string& key = it.key();
				if (key == "intitle")
				{
					if (it.value().is_array())
					{
						for (auto& text : it.value())
						{
							cameraDetectionRule.AddFilter(Rule::FilterType::InTitle, text);
						}
					}
				}
			}
			m_Rules.push_back(cameraDetectionRule);
		}
	}
}

HTTPCameraDetector::Result HTTPCameraDetector::Scan(Network::IPAddress address, CameraDetectionRuleVector const& rules)
{
	CURL* pCurl = curl_easy_init();

	char tagBuffer[32];
	char attrBuffer[32];
	char valBuffer[128];
	char innerTextBuffer[1024];
	HTMLSTREAMPARSER* hsp = html_parser_init();

	html_parser_set_tag_to_lower(hsp, 1);
	html_parser_set_attr_to_lower(hsp, 1);
	html_parser_set_tag_buffer(hsp, tagBuffer, sizeof(tagBuffer));
	html_parser_set_attr_buffer(hsp, attrBuffer, sizeof(attrBuffer));
	html_parser_set_val_buffer(hsp, valBuffer, sizeof(valBuffer) - 1);
	html_parser_set_inner_text_buffer(hsp, innerTextBuffer, sizeof(innerTextBuffer) - 1);

	//CameraScannerCallbackData data = { hsp, this };
	std::string url = "http://" + address.ToString();
	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
	//curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_callback);
	//curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10L);

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);

	Result result;
	result.isCamera = false; //EvaluateDetectionRules(url);
	result.address = address;
	result.title = ""; //m_Title;
	return std::move(result);
}
