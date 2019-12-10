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

GoogleSearch::GoogleSearch():
m_QueryThreadActive(false),
m_QueryThreadStopFlag(false),
m_ShowResultsUI(false)
{
	m_pCurlHandle = curl_easy_init();
	LoadQueries();
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
	return true;
}

void GoogleSearch::LoadQueries()
{
	QueryData data;
	data.query = Query("inurl:/video.mjpg");
	m_QueryDatum.push_back(data);
}

void GoogleSearch::OnMessageReceived(const nlohmann::json& message)
{

}

void GoogleSearch::DrawUI(ImGuiContext* pContext)
{
	ImGui::SetCurrentContext(pContext);

	if (ImGui::CollapsingHeader("Google search", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Queries loaded: %d", m_QueryDatum.size());

		if (ImGui::Button("View results##GoogleSearch"))
		{
			m_ShowResultsUI = !m_ShowResultsUI;
		}

		if (m_ShowResultsUI)
		{
			DrawResultsUI(&m_ShowResultsUI);
		}

		if (IsRunning() == false)
		{
			if (ImGui::Button("Begin queries"))
			{
				Start();
			}
		}
		else
		{
			if (ImGui::Button("Stop queries"))
			{
				Stop();
			}
		}
	}
}

void GoogleSearch::ThreadMain(GoogleSearch* pGoogleSearch)
{
	for (QueryData& queryData : pGoogleSearch->m_QueryDatum)
	{
		do
		{
			std::stringstream url;
			url << "https://www.googleapis.com/customsearch/v1?q=" << queryData.query.GetEncoded() << "&cx=" << sSearchEngineId << "&key=" << sApiKey;

			if (queryData.state.IsValid())
			{
				url << "&start=" << queryData.state.GetCurrentStart();
			}

			CURL* pCurlHandle = pGoogleSearch->m_pCurlHandle;
			char pErrorBuffer[CURL_ERROR_SIZE];
			curl_easy_setopt(pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer);
			curl_easy_setopt(pCurlHandle, CURLOPT_URL, url.str().c_str());
			curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, &pGoogleSearch->m_CurlData);
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
				std::string curlData = pGoogleSearch->m_CurlData;
				std::string filteredCurlData = FilterCurlData(curlData);
				json data = json::parse(filteredCurlData, nullptr, false);

				int startIndex;
				if (ExtractStartIndex(data, startIndex))
				{
					queryData.state.SetCurrentStart(startIndex);
				}

				int totalResults;
				if (ExtractTotalResults(data, totalResults))
				{
					queryData.state.SetResultCount(totalResults);
				}

				{
					std::lock_guard<std::mutex> lock(pGoogleSearch->m_QueryDatumMutex);
					ExtractResults(data, queryData.results);
				}
			}

			// The buffer containing the data from the request needs to be cleared after being used,
			// so further requests have a clean slate.
			pGoogleSearch->m_CurlData.clear();

		} while (queryData.state.IsValid() && !queryData.state.IsCompleted());

		// This can be called without locking as it only reads the results and this thread
		// is the only one which would write to them.
		ProcessResults(pGoogleSearch, queryData);
	}

	pGoogleSearch->m_QueryThreadActive = false;
}

void GoogleSearch::ProcessResults(GoogleSearch* pGoogleSearch, const QueryData& queryData)
{
	for (const QueryResult& result : queryData.results)
	{
		std::string url = result.GetUrl();
		std::size_t start = 0;
		if (url.rfind("https://", 0) == 0)
		{
			start = 8;
		}
		else if (url.rfind("http://", 0) == 0)
		{
			start = 7;
		}
		std::size_t slashPos = url.find_first_of('/', start);
		std::string host = (slashPos == std::string::npos) ? url.substr(start) : url.substr(start, slashPos - start);

		int port = 80;
		int portPos = host.find_first_of(':');
		if (portPos != std::string::npos)
		{
			port = atoi(host.substr(portPos).c_str() + 1);
			host = host.substr(0, portPos);
		}

		Network::IPAddress address;
		if (Network::Resolve(host, address) == Network::Result::Success)
		{
			json message =
			{
				{ "type", "http_server_found" },
				{ "url", url },
				{ "ip_address", address.GetHostAsString() },
				{ "port", port }
			};
			pGoogleSearch->m_pMessageCallback(message);
		}
	}
}

bool GoogleSearch::FilterResult(const QueryData& queryData, const QueryResult& result)
{
	// If our query makes use of inurl:, make sure our result actually has it in the URL.
	// This is necessary as Google will return some results which have something similar
	// in the URL, but are not necessarily exact matches.
	const std::string& encodedQuery = queryData.query.Get();
	std::size_t inUrlStart = encodedQuery.rfind("inurl:", 0);
	if (inUrlStart != std::string::npos)
	{
		std::size_t inUrlEnd = encodedQuery.find_first_of(' ', inUrlStart + 6);
		std::string inUrl = encodedQuery.substr(inUrlStart + 6, inUrlEnd);
		return result.GetUrl().find(inUrl) != std::string::npos;
	}

	return true;
}

std::string GoogleSearch::FilterCurlData(const std::string& data)
{
	size_t dataSize = data.size();
	size_t filteredDataSize = 0u;
	std::string filteredData;
	filteredData.resize(dataSize);
	for (size_t i = 0u; i < dataSize; i++)
	{
		if (data[i] >= 32 && data[i] <= 126)
		{
			filteredData[filteredDataSize++] = data[i];
		}
	}
	filteredData[filteredDataSize] = '\0';
	return filteredData;
}

bool GoogleSearch::ExtractStartIndex(const json& data, int& result)
{
	const json& queries = data["queries"];
	if (queries.is_null() || queries.find("nextPage") == queries.cend())
	{
		return false;
	}

	const json& nextPage = queries["nextPage"];
	if (!nextPage.is_array()) return false;
	for (auto& entry : nextPage)
	{
		const json& startIndex = entry["startIndex"];
		if (startIndex.is_number_integer())
		{
			result = startIndex.get<int>();
			return true;
		}
	}
	return false;
}

bool GoogleSearch::ExtractTotalResults(const json& data, int& result)
{
	const json& queries = data["queries"];
	if (queries.is_null()) return false;
	const json& request = queries["request"];
	if (!request.is_array()) return false;
	for (auto& entry : request)
	{
		// Oddly enough, the "totalResults" entry returned by the API isn't an integer, but a string.
		const json& totalResults = entry["totalResults"];
		if (totalResults.is_string())
		{
			result = atoi(totalResults.get<std::string>().c_str());
			return true;
		}
	}
	return false;
}

bool GoogleSearch::ExtractResults(const json& data, QueryResults& results)
{
	const json& items = data["items"];
	if (!items.is_array()) return false;

	for (auto& item : items)
	{
		std::string url;
		json::const_iterator it;
		if ((it = item.find("link")) != item.end())
		{
			url = it->get<std::string>();
		}

		std::string title;
		if ((it = item.find("title")) != item.end())
		{
			title = it->get<std::string>();
		}

		results.emplace_back(url, title);
	}
	return true;
}

void GoogleSearch::Start()
{
	if (IsRunning() == false)
	{
		m_QueryThreadActive = true;
		m_QueryThreadStopFlag = false;
		m_QueryThread = std::thread(GoogleSearch::ThreadMain, this);
	}
}

void GoogleSearch::Stop()
{
	if (IsRunning())
	{
		m_QueryThreadStopFlag = true;
	}
}

bool GoogleSearch::IsRunning() const
{
	return m_QueryThreadActive;
}

void GoogleSearch::DrawResultsUI(bool* pShow)
{
	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Google Search - results", pShow))
	{
		ImGui::End();
		return;
	}

	std::lock_guard<std::mutex> lock(m_QueryDatumMutex);

	for (auto& data : m_QueryDatum)
	{
		if (ImGui::TreeNode(&data, "(%d) %s", data.results.size(), data.query.Get().c_str()))
		{
			ImGui::Columns(2);
			ImGui::Separator();
			ImGui::Text("URL"); ImGui::NextColumn();
			ImGui::Text("Title"); ImGui::NextColumn();
			ImGui::Separator();

			for (QueryResult result : data.results)
			{
				ImGui::Text("%s", result.GetUrl().c_str()); ImGui::NextColumn();
				ImGui::Text("%s", result.GetTitle().c_str()); ImGui::NextColumn();
			}

			ImGui::Columns(1);
			ImGui::TreePop();
		}
	}

	ImGui::End();
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
