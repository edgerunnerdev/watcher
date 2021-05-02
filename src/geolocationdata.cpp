#include "database/database.h"
#include "json.h"
#include "geolocationdata.h"

GeolocationData::GeolocationData( Network::IPAddress address ) :
m_Address( address ),
m_Latitude( 0.0f ),
m_Longitude( 0.0f )
{
}

bool GeolocationData::LoadFromDatabase( const std::string& city, const std::string& region, const std::string& country, const std::string& organisation, float latitude, float longitude )
{
	m_City = city;
	m_Region = region;
	m_Country = country;
	m_Organisation = organisation;
	m_Latitude = latitude;
	m_Longitude = longitude;
	return true;
}

bool GeolocationData::LoadFromJSON( const json& geolocationDataJson )
{
	m_City = geolocationDataJson[ "city" ];
	m_Region = geolocationDataJson[ "region" ];
	m_Country = geolocationDataJson[ "country" ];
	m_Organisation = geolocationDataJson[ "org" ];

	std::string location = geolocationDataJson[ "loc" ];
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

void GeolocationData::SaveToDatabase( Database::Database* pDatabase )
{
	Database::PreparedStatement addGeolocationStatement( pDatabase, "INSERT OR REPLACE INTO Geolocation VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7);" );
	addGeolocationStatement.Bind( 1, m_Address.ToString() );
	addGeolocationStatement.Bind( 2, m_City );
	addGeolocationStatement.Bind( 3, m_Region );
	addGeolocationStatement.Bind( 4, m_Country );
	addGeolocationStatement.Bind( 5, m_Organisation );
	addGeolocationStatement.Bind( 6, static_cast< double >( m_Latitude ) );
	addGeolocationStatement.Bind( 7, static_cast< double >( m_Longitude ) );
	pDatabase->Execute( addGeolocationStatement );

	Database::PreparedStatement updateCameraStatement( pDatabase, "UPDATE Cameras SET Geolocated=1 WHERE IP=?1;" );
	updateCameraStatement.Bind( 1, m_Address.ToString() );
	pDatabase->Execute( updateCameraStatement );
}
