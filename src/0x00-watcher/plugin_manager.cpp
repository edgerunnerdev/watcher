#include <Windows.h>
#include "plugin.h"
#include "plugin_manager.h"

using PluginInitialiseFnPtr = int (*)();

PluginManager::PluginManager()
{
	HINSTANCE pluginDll = LoadLibrary( "plugins\\geolocation.dll" );
	if ( pluginDll )
	{
		PluginInitialiseFnPtr pluginInitialiseFn = (PluginInitialiseFnPtr)GetProcAddress( pluginDll, "GetPluginVersion" );
		if ( pluginInitialiseFn )
		{
			int version = pluginInitialiseFn();
			int a = 0;
		}
		else
		{
			int b = 0;
		}
	}
}