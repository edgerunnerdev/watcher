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
//#include <mutex>
//#include <thread>
//#include <vector>
//
//using CURL = void;
//
//class Geolocation : public Plugin
//{
//	DECLARE_PLUGIN( Geolocation, 0, 3, 0 );
//public:
//	Geolocation();
//	virtual ~Geolocation();
//	virtual bool Initialise( PluginMessageCallback pMessageCallback ) override;
//	virtual void OnMessageReceived( const nlohmann::json& message ) override;
//	virtual void DrawUI( ImGuiContext* pContext ) override;
//
//private:
//	void ConsumeQueue();
//
//	PluginMessageCallback m_pMessageCallback;
//	std::mutex m_AccessMutex;
//	std::vector< Network::IPAddress > m_Queue;
//	std::thread m_QueryThread;
//	std::atomic_bool m_QueryThreadActive;
//	std::string m_Data;
//	bool m_RateLimitExceeded;
//};