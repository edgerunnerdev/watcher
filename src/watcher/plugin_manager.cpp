#ifdef _WIN32
#include <Windows.h>
#elif defined __linux__
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#endif
#include "log.h"
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

// Return the DLLs / SOs for every plugin we want to load.
// Each plugin has its own folder inside the "plugins" folder.
PluginManager::SharedLibraryPaths PluginManager::DiscoverSharedLibraries()
{
	SharedLibraryPaths paths;

#ifdef _WIN32
	// Find all the folders inside the "plugins" folder.
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile( "plugins\\*", &data );
	std::vector< std::string > pluginDirectories;
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do 
		{
			if ( ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 && data.cFileName[0] != '.')
			{
				pluginDirectories.push_back( "plugins\\" + std::string( data.cFileName ) + "\\" );
			}

		} while ( FindNextFile( hFind, &data ) );
	}
	FindClose( hFind );

	// Try to find a shared library in each potential plugin directory.
	for ( const std::string& pluginDirectory : pluginDirectories ) 
	{
		std::string pluginFolderSearch = pluginDirectory + "*.dll";
		hFind = FindFirstFile( pluginFolderSearch.c_str(), &data );
		if ( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				if ( ( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
				{
					paths.push_back( pluginDirectory + "\\" + data.cFileName );
					Log::Info( "Plugin found: %s", paths.back().c_str() );
				}
			} while ( FindNextFile( hFind, &data ) );
		}
		FindClose( hFind );
	}

#elif defined __linux__
	auto hasExtensionFn = []( const std::string& filename, const std::string& extension ) -> bool {
		if ( filename.size() >= extension.size() && filename.compare( filename.size() - extension.size(), extension.size(), extension) == 0 )
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	DIR* dirp = opendir( "plugins" );
	struct dirent* dp;
	while ( ( dp = readdir( dirp ) ) != nullptr ) 
	{
		if ( hasExtensionFn( dp->d_name, ".so" ) )
		{
			paths.push_back( pluginsFolder + "/" + dp->d_name );
			Log::Info( "Plugin found: %s", paths.back().c_str() );
		}
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
			getPluginFnPtr = reinterpret_cast< GetPluginFnPtr >( GetProcAddress( pluginDll, "GetPlugin" ) );
			Log::Info( "Plugin loaded: %s", sharedLibraryPath.c_str() );
		}
#elif defined __linux__
		Log::Info( "Trying to open: %s", sharedLibraryPath.c_str() );
		void* handle = dlopen( sharedLibraryPath.c_str() , RTLD_NOW );
		if ( handle )
		{
			getPluginFnPtr = reinterpret_cast< GetPluginFnPtr >( dlsym( handle, "GetPlugin" ) );
			Log::Info( "Plugin loaded: %s", sharedLibraryPath.c_str() );
		}
		else
		{
			Log::Warning( "%s", dlerror() );
		}
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