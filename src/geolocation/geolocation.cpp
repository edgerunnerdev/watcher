#include "stdafx.h"
#include "geolocation.h"

DECLARE_PLUGIN( Geolocation, 0, 1, 0 )

bool Geolocation::Initialise()
{
	return true;
}

void Geolocation::OnMessage( const nlohmann::json& message )
{

}