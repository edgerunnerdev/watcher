// geolocation.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "geolocation.h"

DECLARE_PLUGIN( Geolocation )

bool Geolocation::Initialise()
{
	return true;
}

void Geolocation::OnMessage( const nlohmann::json& message )
{

}