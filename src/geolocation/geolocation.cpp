// geolocation.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "geolocation.h"

int PLUGINAPI GetPluginVersion()
{
	return 1;
}

bool Geolocation::Initialise()
{
	return true;
}

void OnMessage( const nlohmann::json& message )
{

}