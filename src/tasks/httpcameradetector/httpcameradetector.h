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
//#pragma once
//
//#include <atomic>
//#include <deque>
//#include <mutex>
//#include <thread>
//#include <vector>
//
//#include "../watcher/plugin.h"
//#include "network/network.h"
//#include "threadpool.h"
//
//class Rule;
//using RuleVector = std::vector<Rule>;
//
//class HTTPCameraDetector : public Plugin
//{
//	DECLARE_PLUGIN(HTTPCameraDetector, 0, 1, 0);
//public:
//	HTTPCameraDetector();
//	virtual ~HTTPCameraDetector();
//	virtual bool Initialise(PluginMessageCallback pMessageCallback) override;
//	virtual void OnMessageReceived(const nlohmann::json& message) override;
//	virtual void DrawUI(ImGuiContext* pContext) override;
//	static void Scan(HTTPCameraDetector* pDetector, const std::string& url, const std::string& ipAddress, int port);
//
//private:
//	void LoadRules();
//	void DrawResultsUI(bool* pShow);
//	static bool EvaluateDetectionRules(HTTPCameraDetector* pDetector, const std::string& url, const std::string& title);
//	PluginMessageCallback m_pMessageCallback;
//	ThreadPool m_ThreadPool;
//	std::atomic_int m_PendingResults;
//
//	struct Result
//	{
//		bool isCamera;
//		std::string url;
//		std::string title;
//	};
//	std::mutex m_ResultsMutex;
//	std::deque<Result> m_Results;
//	bool m_ShowResultsUI;
//
//	RuleVector m_Rules;
//};
