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
	DECLARE_PLUGIN(Portscanner, 0, 1, 0);
public:
	Portscanner();
	virtual ~Portscanner();
	virtual bool Initialise(PluginMessageCallback pMessageCallback) override;
	virtual void OnMessageReceived(const nlohmann::json& message) override;
	virtual void DrawUI(ImGuiContext* pContext) override;

private:
	PluginMessageCallback m_pMessageCallback;
};