#pragma once

#include "network/network.h"

struct sqlite3;

class GeoInfo
{
public:
	GeoInfo( Network::IPAddress address );
	bool LoadFromDatabase( const std::string& city, const std::string& region, const std::string& country, const std::string& organisation, float x, float y );
	bool LoadFromJSON( const std::string& data );
	void SaveToDatabase( sqlite3* pDatabase );

	const Network::IPAddress& GetIPAddress() const;
	const std::string& GetCity() const;
	const std::string& GetRegion() const;
	const std::string& GetCountry() const;
	const std::string& GetOrganisation() const;
	void GetLocation( float& x, float& y ) const;

private:
	Network::IPAddress m_Address;
	std::string m_City;
	std::string m_Region;
	std::string m_Country;
	std::string m_Organisation;
	float m_Location[ 2 ];
};

inline const Network::IPAddress& GeoInfo::GetIPAddress() const
{
	return m_Address;
}

inline const std::string& GeoInfo::GetCity() const
{
	return m_City;
}

inline const std::string& GeoInfo::GetRegion() const
{
	return m_Region;
}

inline const std::string& GeoInfo::GetCountry() const
{
	return m_Country;
}

inline const std::string& GeoInfo::GetOrganisation() const
{
	return m_Organisation;
}

inline void GeoInfo::GetLocation( float &x, float &y ) const
{
	x = m_Location[ 0 ];
	y = m_Location[ 1 ];
}
