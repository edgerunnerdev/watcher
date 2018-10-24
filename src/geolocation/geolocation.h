#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "..\0x00-watcher\plugin.h"
#include "network/network.h"

using CURL = void;

class Geolocation : public Plugin
{
public:
	Geolocation();
	virtual ~Geolocation();
	bool Initialise( PluginMessageCallback pMessageCallback ) override;
	void OnMessageReceived( const nlohmann::json& message ) override;

private:
	void ConsumeQueue();

	PluginMessageCallback m_pMessageCallback;
	std::mutex m_QueueMutex;
	std::vector< Network::IPAddress > m_Queue;
	std::thread m_QueryThread;
	std::atomic_bool m_QueryThreadActive;
	CURL* m_pCurlHandle;
};