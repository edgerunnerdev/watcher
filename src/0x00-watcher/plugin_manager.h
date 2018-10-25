#pragma once

#include <string>
#include <vector>
#include "json.h"

class Plugin;

struct PluginData
{
	Plugin* pPlugin;
	std::string name;
	int versionMajor;
	int versionMinor;
	int versionPatch;
};
using PluginDataVector = std::vector< PluginData >;

class PluginManager
{
public:
	PluginManager();
	void BroadcastMessage( const nlohmann::json& message );
	const PluginDataVector& GetPlugins() const { return m_Plugins; }

private:
	using SharedLibraryPaths = std::vector< std::string >;
	SharedLibraryPaths DiscoverSharedLibraries();

	void LoadPlugins( const SharedLibraryPaths& sharedLibraryPaths );
	void InitialisePlugins();

	PluginDataVector m_Plugins;
};