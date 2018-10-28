#ifdef _WIN32
#include <Windows.h>
#elif defined __linux__
#include <sys/types.h>
#include <dirent.h>
#endif
#include "plugin.h"
#include "plugin_manager.h"
#include "watcher.h"

extern Watcher* g_pWatcher;
void WatcherMessageCallback( const json& message )
{
	g_pWatcher->OnMessageReceived( message );
}

PluginManager::PluginManager()
{
	SharedLibraryPaths sharedLibraryPaths;
	sharedLibraryPaths = DiscoverSharedLibraries();
	LoadPlugins( sharedLibraryPaths );
	InitialisePlugins();
}

PluginManager::SharedLibraryPaths PluginManager::DiscoverSharedLibraries()
{
	SharedLibraryPaths paths;
	std::string pluginsFolder( "plugins" );

#ifdef _WIN32
	HANDLE hFind;
	WIN32_FIND_DATA data;
	for ( hFind = FindFirstFile( "plugins\\*.dll", &data ); hFind != INVALID_HANDLE_VALUE; FindNextFile( hFind, &data ) )
	{
		if ( GetLastError() == ERROR_NO_MORE_FILES )
		{
			break;
		}
		else if ( ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
		{
			paths.push_back( pluginsFolder + "\\" + data.cFileName );
		}
	}
	FindClose( hFind );
#elif defined __linux__
	DIR* dirp = opendir( "plugins" );
	struct dirent* dp;
	while ( ( dp = readdir( dirp ) ) != nullptr ) 
	{
		paths.push_back( pluginsFolder + "/" + dp->d_name );
	}
	closedir( dirp );
#else
	static_assert( false, "" ); // Not implemented.
#endif

	return paths;
}

void PluginManager::LoadPlugins( const SharedLibraryPaths& sharedLibraryPaths )
{
	using GetPluginFnPtr = Plugin * (*)();

	m_Plugins.reserve( sharedLibraryPaths.size() );

	for ( const std::string& sharedLibraryPath : sharedLibraryPaths )
	{
		GetPluginFnPtr getPluginFnPtr = nullptr;

#ifdef _WIN32
		HINSTANCE pluginDll = LoadLibrary( sharedLibraryPath.c_str() );
		if ( pluginDll )
		{
			getPluginFnPtr = (GetPluginFnPtr)GetProcAddress( pluginDll, "GetPlugin" );
		}
#elif defined __linux__

#else
		static_assert( false, "" ); // Not implemented
#endif

		if ( getPluginFnPtr != nullptr )
		{
			m_Plugins.push_back( getPluginFnPtr() );
		}
	}
}

void PluginManager::InitialisePlugins()
{
	for ( Plugin* pPlugin : m_Plugins )
	{
		pPlugin->Initialise( &WatcherMessageCallback );
	}
}

void PluginManager::BroadcastMessage( const nlohmann::json& message )
{
	for ( Plugin* pPlugin : m_Plugins )
	{
		pPlugin->OnMessageReceived( message );
	}
}