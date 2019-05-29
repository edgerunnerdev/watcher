#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "../watcher/plugin.h"
#include "network/network.h"

using CURL = void;

class Portscanner : public Plugin
{
	DECLARE_PLUGIN(Portscanner, 0, 2, 0);
public:
	Portscanner();
	virtual ~Portscanner();
	virtual bool Initialise(PluginMessageCallback pMessageCallback) override;
	virtual void OnMessageReceived(const nlohmann::json& message) override;
	virtual void DrawUI(ImGuiContext* pContext) override;

private:
	void ConsumeQueue();

	PluginMessageCallback m_pMessageCallback;
	std::mutex m_AccessMutex;
	std::vector< Network::IPAddress > m_Queue;
	std::thread m_QueryThread;
	std::atomic_bool m_QueryThreadActive;
	CURL* m_pCurlHandle;
	std::string m_Data;
};