#include "sqlite/sqlite3.h"
#include "database_helpers.h"
#include "json.h"
#include "geo_info.h"

GeoInfo::GeoInfo( Network::IPAddress address ) :
m_Address( address ),
m_Latitude( 0.0f ),
m_Longitude( 0.0f )
{
}

bool GeoInfo::LoadFromDatabase( const std::string& city, const std::string& region, const std::string& country, const std::string& organisation, float latitude, float longitude )
{
	m_City = city;
	m_Region = region;
	m_Country = country;
	m_Organisation = organisation;
	m_Latitude = latitude;
	m_Longitude = longitude;
	return true;
}

bool GeoInfo::LoadFromJSON( const json& geoInfoJson )
{
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
		std::string latitude = location.substr( 0, locationSeparator );
		std::string longitude = location.substr( locationSeparator + 1 );
		m_Latitude = static_cast< float >( atof( latitude.c_str() ) );
		m_Longitude = static_cast< float >( atof( longitude.c_str() ) );
		return true;
	}
}

void GeoInfo::SaveToDatabase( sqlite3* pDatabase )
{
	sqlite3_stmt* pAddGeoInfoStatement;
	const char* pAddGeoInfoQuery = "INSERT OR REPLACE INTO Geolocation VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7);";
	sqlite3_prepare_v2( pDatabase, pAddGeoInfoQuery, -1, &pAddGeoInfoStatement, nullptr );

	sqlite3_bind_text( pAddGeoInfoStatement, 1, m_Address.ToString().c_str(), -1, SQLITE_TRANSIENT );
	sqlite3_bind_text( pAddGeoInfoStatement, 2, m_City.c_str(), -1, SQLITE_TRANSIENT );
	sqlite3_bind_text( pAddGeoInfoStatement, 3, m_Region.c_str(), -1, SQLITE_TRANSIENT );
	sqlite3_bind_text( pAddGeoInfoStatement, 4, m_Country.c_str(), -1, SQLITE_TRANSIENT );
	sqlite3_bind_text( pAddGeoInfoStatement, 5, m_Organisation.c_str(), -1, SQLITE_TRANSIENT );
	sqlite3_bind_double( pAddGeoInfoStatement, 6, (double)m_Latitude );
	sqlite3_bind_double( pAddGeoInfoStatement, 7, (double)m_Longitude );
	ExecuteDatabaseQuery( pDatabase, pAddGeoInfoStatement );
	sqlite3_finalize( pAddGeoInfoStatement );

	std::stringstream updateCameraQuery;
	updateCameraQuery << "UPDATE Cameras SET Geo=1 WHERE IP='" << m_Address.ToString() << "';";
	ExecuteDatabaseQuery( pDatabase, updateCameraQuery.str() );
}
