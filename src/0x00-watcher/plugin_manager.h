#pragma once

#include <string>
#include <vector>
#include "json.h"

class Plugin;

class PluginManager
{
public:
	PluginManager();
	void BroadcastMessage( const nlohmann::json& message );

private:
	using SharedLibraryPaths = std::vector< std::string >;
	SharedLibraryPaths DiscoverSharedLibraries();

	void LoadPlugins( const SharedLibraryPaths& sharedLibraryPaths );

	struct PluginData
	{
		Plugin* pPlugin;
		std::string name;
		int versionMajor;
		int versionMinor;
		int versionPatch;
	};
	using PluginDataVector = std::vector< PluginData >;
	PluginDataVector m_Plugins;
};