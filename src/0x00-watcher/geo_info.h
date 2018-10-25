#pragma once

#include "network/network.h"
#include "json.h"
using json = nlohmann::json;

struct sqlite3;

class GeoInfo
{
public:
	GeoInfo( Network::IPAddress address );
	bool LoadFromDatabase( const std::string& city, const std::string& region, const std::string& country, const std::string& organisation, float x, float y );
	bool LoadFromJSON( const json& message );
	void SaveToDatabase( sqlite3* pDatabase );

	const Network::IPAddress& GetIPAddress() const;
	const std::string& GetCity() const;
	const std::string& GetRegion() const;
	const std::string& GetCountry() const;
	const std::string& GetOrganisation() const;
	float GetLatitude() const;
	float GetLongitude() const;

private:
	Network::IPAddress m_Address;
	std::string m_City;
	std::string m_Region;
	std::string m_Country;
	std::string m_Organisation;
	float m_Latitude;
	float m_Longitude;
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

inline float GeoInfo::GetLatitude() const
{
	return m_Latitude;
}

inline float GeoInfo::GetLongitude() const
{
	return m_Longitude;
}