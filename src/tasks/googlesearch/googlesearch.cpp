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

#include <chrono>

#include "googlesearch.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <curl/curl.h>
#include <imgui/imgui.h>

#include <stdio.h>

#include "database/databasetime.h"
#include "configuration.h"
#include "encoding.h"
#include "log.h"
#include "watcher.h"

namespace Watcher
{
namespace Tasks
{

// API documentation: https://developers.google.com/custom-search/v1/cse/list

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
SearchTask("Google search", "queries/googlecse.json"),
m_QueryThreadActive(true),
m_QueryThreadStopFlag(false),
m_ShowResultsUI(false),
m_IsRunning(false)
{
	Enable();

	m_QueryThread = std::thread(GoogleSearch::ThreadMain, this);
}

GoogleSearch::~GoogleSearch()
{
	m_QueryThreadStopFlag = true;
	if (m_QueryThread.joinable())
	{
		m_QueryThread.join();
	}
}

void GoogleSearch::Update(float delta)
{
	SearchTask::Update(delta);
}

void GoogleSearch::ThreadMain(GoogleSearch* pGoogleSearch)
{
	CURL* pCurlHandle = curl_easy_init();
	
	while (pGoogleSearch->m_QueryThreadStopFlag == false)
	{
		if (pGoogleSearch->IsRunning())
		{
			std::vector<SearchTask::QueryData> pendingQueries = pGoogleSearch->FindPendingQueries();
			if (pendingQueries.size() > 0)
			{
				pGoogleSearch->SetState(State::Running);
				
				for (auto& queryData : pendingQueries)
				{
					pGoogleSearch->ExecuteQuery(queryData.query);
				}
			}
			else
			{
				pGoogleSearch->SetState(State::Idle);
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	curl_easy_cleanup(pCurlHandle);
	pGoogleSearch->m_QueryThreadActive = false;
}

std::vector<SearchTask::QueryData> GoogleSearch::FindPendingQueries() const
{
	std::vector<QueryData> pendingQueries;
	for (auto& queryData : m_Queries)
	{
		if (queryData.time.Get().has_value() == false || queryData.time.Get().value() + std::chrono::hours(24*14) < std::chrono::system_clock::now())
		{
			pendingQueries.push_back(queryData);
		}
	}

	return pendingQueries;
}

void GoogleSearch::ProcessResults(GoogleSearch* pGoogleSearch, const QueryData& queryData)
{
	//for (const QueryResult& result : queryData.results)
	//{
	//	std::string url = result.GetUrl();
	//	std::size_t start = 0;
	//	if (url.rfind("https://", 0) == 0)
	//	{
	//		start = 8;
	//	}
	//	else if (url.rfind("http://", 0) == 0)
	//	{
	//		start = 7;
	//	}
	//	std::size_t slashPos = url.find_first_of('/', start);
	//	std::string host = (slashPos == std::string::npos) ? url.substr(start) : url.substr(start, slashPos - start);

	//	int port = 80;
	//	int portPos = host.find_first_of(':');
	//	if (portPos != std::string::npos)
	//	{
	//		port = atoi(host.substr(portPos).c_str() + 1);
	//		host = host.substr(0, portPos);
	//	}

	//	//Network::IPAddress address;
	//	//if (Network::Resolve(host, address) == Network::Result::Success)
	//	//{
	//	//	//json message =
	//	//	//{
	//	//	//	{ "type", "http_server_found" },
	//	//	//	{ "url", url },
	//	//	//	{ "ip_address", address.GetHostAsString() },
	//	//	//	{ "port", port }
	//	//	//};
	//	//	//pGoogleSearch->m_pMessageCallback(message);
	//	//}
	//}
}

bool GoogleSearch::FilterResult(const QueryData& queryData, const QueryResult& result)
{
	// If our query makes use of inurl:, make sure our result actually has it in the URL.
	// This is necessary as Google will return some results which have something similar
	// in the URL, but are not necessarily exact matches.
	//const std::string& encodedQuery = queryData.query.Get();
	//std::size_t inUrlStart = encodedQuery.rfind("inurl:", 0);
	//if (inUrlStart != std::string::npos)
	//{
	//	std::size_t inUrlEnd = encodedQuery.find_first_of(' ', inUrlStart + 6);
	//	std::string inUrl = encodedQuery.substr(inUrlStart + 6, inUrlEnd);
	//	return result.GetUrl().find(inUrl) != std::string::npos;
	//}

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

void GoogleSearch::ExecuteQuery(const std::string& query)
{
	Configuration* pConfiguration = g_pWatcher->GetConfiguration();
	CURL* pCurlHandle = curl_easy_init();
	if (pCurlHandle == nullptr)
	{
		Log::Error("Couldn't initialise CURL handle.");
		return;
	}

	QueryState state;
	QueryResults results;
	do
	{
		std::stringstream url;
		url << "https://www.googleapis.com/customsearch/v1?q=" << Encoding::URLEncode(query) << "&cx=" << pConfiguration->GetGoogleCSEId() << "&key=" << pConfiguration->GetGoogleCSEApiKey();

		if (state.IsValid())
		{
			url << "&start=" << state.GetCurrentStart();
		}

		Log::Info("[GoogleSearch] %s", url.str().c_str());

		char pErrorBuffer[CURL_ERROR_SIZE];
		curl_easy_setopt(pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer);
		curl_easy_setopt(pCurlHandle, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, &m_CurlData);
		curl_easy_setopt(pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(pCurlHandle, CURLOPT_CAINFO, "cacert.pem");
		curl_easy_setopt(pCurlHandle, CURLOPT_TIMEOUT, 10L);

		if (curl_easy_perform(pCurlHandle) != CURLE_OK)
		{
			Log::Error("GoogleSearch error: %s.", pErrorBuffer);
			//pGoogleSearch->SetState(Task::State::Error);
			//pGoogleSearch->SetErrorString(pErrorBuffer);
		}
		else
		{
			std::string filteredCurlData = FilterCurlData(m_CurlData);
			json data = json::parse(filteredCurlData, nullptr, false);

			json::iterator errorIt = data.find("error");
			if (errorIt != data.end())
			{
				//pGoogleSearch->SetState(Task::State::Error);
				std::string error = (*errorIt)["status"].get<std::string>();
				if (error == "RESOURCE_EXHAUSTED")
				{
					SetErrorString("Quota exceeded");
				}
				else
				{
					SetErrorString(error);
				}
			}
			else
			{

				int startIndex;
				if (ExtractStartIndex(data, startIndex))
				{
					state.SetCurrentStart(startIndex);
				}

				int totalResults;
				if (ExtractTotalResults(data, totalResults))
				{
					state.SetResultCount(totalResults);
				}

				{
					//std::lock_guard<std::mutex> lock(pGoogleSearch->m_QueryDatumMutex);
					ExtractResults(data, results);
				}
			}
		}

		// The buffer containing the data from the request needs to be cleared after being used,
		// so further requests have a clean slate.
		m_CurlData.clear();

		// Break for debugging purposes
		break;

	} while (state.IsValid() && !state.IsCompleted() && !m_QueryThreadStopFlag);

	//ProcessResults(pGoogleSearch, queryData);
	curl_easy_cleanup(pCurlHandle);
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
		if (entry.find("totalResults") == entry.cend())
		{
			return false;
		}

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
	if (data.find("items") == data.end())
	{
		return false;
	}

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
	SearchTask::Start();

	if (GetState() == Task::State::Disabled)
	{
		return;
	}

	m_IsRunning = true;
}

void GoogleSearch::Stop()
{
	SearchTask::Stop();
	m_IsRunning = false;
}

void GoogleSearch::ShowQueriesUI(bool* pOpen)
{
	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Google search queries", pOpen))
	{
		ImGui::Columns(2);
		ImGui::Text("Query"); ImGui::NextColumn();
		ImGui::Text("Date"); ImGui::NextColumn();

		for (auto& queryData : m_Queries)
		{
			ImGui::Text("%s", queryData.query.c_str()); ImGui::NextColumn();
			ImGui::Text("%s", queryData.time.GetAsString().c_str()); ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::End();
	}
}

bool GoogleSearch::IsRunning() const
{
	return m_IsRunning;
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

} // namespace Tasks
} // namespace Watcher
