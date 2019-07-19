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

#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "../watcher/plugin.h"
#include "network/network.h"
#include "query.h"

using CURL = void;

class GoogleSearch : public Plugin
{
	DECLARE_PLUGIN(GoogleSearch, 0, 1, 0);
public:
	GoogleSearch();
	virtual ~GoogleSearch();
	virtual bool Initialise(PluginMessageCallback pMessageCallback) override;
	virtual void OnMessageReceived(const nlohmann::json& message) override;
	virtual void DrawUI(ImGuiContext* pContext) override;

	void Start();
	void Stop();

private:
	static void ThreadMain(GoogleSearch* pGoogleSearch);
	static std::string FilterCurlData(const std::string& data);
	static bool ExtractStartIndex(const json& data, int& result);
	static bool ExtractTotalResults(const json& data, int& result);
	static bool ExtractResults(const json& data, QueryResults& results);
	static void ProcessResults(GoogleSearch* pGoogleSearch, const QueryData& queryData);
	static bool FilterResult(const QueryData& queryData, const QueryResult& result);
	bool IsRunning() const;
	void LoadQueries();
	void DrawResultsUI(bool* pShow);

	PluginMessageCallback m_pMessageCallback;
	std::thread m_QueryThread;
	std::atomic_bool m_QueryThreadActive;
	std::atomic_bool m_QueryThreadStopFlag;
	bool m_ShowResultsUI;
	CURL* m_pCurlHandle;
	std::string m_CurlData;
	QueryDatum m_QueryDatum;
	std::mutex m_QueryDatumMutex;
};
