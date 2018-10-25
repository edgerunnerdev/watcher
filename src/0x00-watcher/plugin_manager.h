#pragma once

#include <string>
#include <vector>
#include "json.h"

class Plugin;
using PluginVector = std::vector< Plugin* >;

class PluginManager
{
public:
	PluginManager();
	void BroadcastMessage( const nlohmann::json& message );
	const PluginVector& GetPlugins() const { return m_Plugins; }

private:
	using SharedLibraryPaths = std::vector< std::string >;
	SharedLibraryPaths DiscoverSharedLibraries();

	void LoadPlugins( const SharedLibraryPaths& sharedLibraryPaths );
	void InitialisePlugins();

	PluginVector m_Plugins;
};