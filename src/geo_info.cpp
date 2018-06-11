#include "sqlite/sqlite3.h"
#include "database_helpers.h"
#include "json.h"
#include "geo_info.h"

GeoInfo::GeoInfo( Network::IPAddress address ) :
m_Address( address )
{
	m_Location[ 0 ] = 0.0f;
	m_Location[ 1 ] = 0.0f;
}

bool GeoInfo::LoadFromDatabase( const std::string& city, const std::string& region, const std::string& country, const std::string& organisation, float x, float y )
{
	m_City = city;
	m_Region = region;
	m_Country = country;
	m_Organisation = organisation;
	m_Location[ 0 ] = x;
	m_Location[ 1 ] = y;
	return true;
}

bool GeoInfo::LoadFromJSON( const std::string& data )
{
	using json = nlohmann::json;
	json geoInfoJson = json::parse( data );
	m_City = geoInfoJson[ "city" ];
	m_Region = geoInfoJson[ "region" ];
	m_Country = geoInfoJson[ "country" ];
	m_Organisation = geoInfoJson[ "org" ];

	std::string location = geoInfoJson[ "loc" ];
	size_t locationSeparator = location.find_first_of(',');
	if ( locationSeparator == std::string::npos )
	{
		return false;
	}
	else
	{
		std::string x = location.substr( 0, locationSeparator );
		std::string y = location.substr( locationSeparator + 1 );
		m_Location[ 0 ] = static_cast< float >( atof( x.c_str() ) );
		m_Location[ 1 ] = static_cast< float >( atof( y.c_str() ) );
		return true;
	}
}

void GeoInfo::SaveToDatabase( sqlite3* pDatabase )
{
	std::stringstream addGeoInfoQuery;
	addGeoInfoQuery << "INSERT OR REPLACE INTO Geolocation VALUES(" <<
		"'" << m_Address.ToString() << "', " <<
		"'" << m_City << "', " <<
		"'" << m_Region << "', " <<
		"'" << m_Country << "', " <<
		"'" << m_Organisation << "', " <<
		m_Location[ 0 ] << ", " <<
		m_Location[ 1 ] << ");";
	ExecuteDatabaseQuery( pDatabase, addGeoInfoQuery.str() );

	std::stringstream updateCameraQuery;
	updateCameraQuery << "UPDATE Cameras SET Geo=1 WHERE IP='" << m_Address.ToString() << "';";
	ExecuteDatabaseQuery( pDatabase, updateCameraQuery.str() );
}
