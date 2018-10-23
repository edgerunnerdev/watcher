#ifdef _WIN32
#include <Windows.h>
#endif
#include "plugin.h"
#include "plugin_manager.h"


PluginManager::PluginManager()
{
	SharedLibraryPaths sharedLibraryPaths;
	sharedLibraryPaths = DiscoverSharedLibraries();
	LoadPlugins( sharedLibraryPaths );
}

PluginManager::SharedLibraryPaths PluginManager::DiscoverSharedLibraries()
{
	SharedLibraryPaths paths;

#ifdef _WIN32
	HANDLE hFind;
	WIN32_FIND_DATA data;
	std::string pluginsFolder( "plugins\\" );
	for ( hFind = FindFirstFile( "plugins\\*.dll", &data ); hFind != INVALID_HANDLE_VALUE; FindNextFile( hFind, &data ) )
	{
		if ( GetLastError() == ERROR_NO_MORE_FILES )
		{
			break;
		}
		else if ( ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
		{
			paths.push_back( pluginsFolder + data.cFileName );
		}
	}
	FindClose( hFind );
#else
	static_assert( false ); // Not implemented.
#endif

	return paths;
}

void PluginManager::LoadPlugins( const SharedLibraryPaths& sharedLibraryPaths )
{
	using GetPluginFnPtr = Plugin * (*)();
	using GetPluginNameFnPtr = const char* (*)();
	using GetPluginVersionFnPtr = void(*)( int& major, int& minor, int& patch );

	m_Plugins.reserve( sharedLibraryPaths.size() );

	for ( const std::string& sharedLibraryPath : sharedLibraryPaths )
	{
		GetPluginFnPtr getPluginFnPtr = nullptr;
		GetPluginNameFnPtr getPluginNameFnPtr = nullptr;
		GetPluginVersionFnPtr getPluginVersionFnPtr = nullptr;

#ifdef _WIN32
		HINSTANCE pluginDll = LoadLibrary( sharedLibraryPath.c_str() );
		if ( pluginDll )
		{
			getPluginFnPtr = (GetPluginFnPtr)GetProcAddress( pluginDll, "GetPlugin" );
			getPluginNameFnPtr = (GetPluginNameFnPtr)GetProcAddress( pluginDll, "GetPluginName" );
			getPluginVersionFnPtr = (GetPluginVersionFnPtr)GetProcAddress( pluginDll, "GetPluginVersion" );
		}
#else
		static_assert( false );
#endif

		if ( getPluginFnPtr != nullptr && getPluginNameFnPtr != nullptr && getPluginVersionFnPtr != nullptr )
		{
			PluginData pluginData;
			pluginData.pPlugin = getPluginFnPtr();
			pluginData.name = std::string( getPluginNameFnPtr() );
			getPluginVersionFnPtr( pluginData.versionMajor, pluginData.versionMinor, pluginData.versionPatch );
			m_Plugins.push_back( pluginData );
		}
	}
}

void PluginManager::BroadcastMessage( const nlohmann::json& message )
{
	for ( auto& pluginData : m_Plugins )
	{
		pluginData.pPlugin->OnMessage( message );
	}
}