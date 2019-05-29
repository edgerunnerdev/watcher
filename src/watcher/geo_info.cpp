#include "database/database.h"
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

void GeoInfo::SaveToDatabase( Database::Database* pDatabase )
{
	Database::PreparedStatement addGeoInfoStatement( pDatabase, "INSERT OR REPLACE INTO Geolocation VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7);" );
	addGeoInfoStatement.Bind( 1, m_Address.ToString() );
	addGeoInfoStatement.Bind( 2, m_City );
	addGeoInfoStatement.Bind( 3, m_Region );
	addGeoInfoStatement.Bind( 4, m_Country );
	addGeoInfoStatement.Bind( 5, m_Organisation );
	addGeoInfoStatement.Bind( 6, static_cast< double >( m_Latitude ) );
	addGeoInfoStatement.Bind( 7, static_cast< double >( m_Longitude ) );
	pDatabase->Execute( addGeoInfoStatement );

	Database::PreparedStatement updateCameraStatement( pDatabase, "UPDATE Cameras SET Geo=1 WHERE IP=?1;" );
	updateCameraStatement.Bind( 1, m_Address.ToString() );
	pDatabase->Execute( updateCameraStatement );
}
