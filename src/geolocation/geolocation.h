#pragma once

#include "..\0x00-watcher\plugin.h"

class Geolocation : public Plugin
{
public:
	bool Initialise( PluginMessageCallback pMessageCallback ) override;
	void OnMessage( const nlohmann::json& message ) override;

private:
	PluginMessageCallback m_pMessageCallback;
};