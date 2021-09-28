///////////////////////////////////////////////////////////////////////////////
// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#include "database/database.h"
#include "json.h"
#include "geolocationdata.h"

namespace Watcher
{

GeolocationData::GeolocationData(IPAddress address) :
	m_Address(address),
	m_Latitude(0.0f),
	m_Longitude(0.0f)
{
}

bool GeolocationData::LoadFromDatabase(const std::string& city, const std::string& region, const std::string& country, const std::string& organisation, float latitude, float longitude)
{
	m_City = city;
	m_Region = region;
	m_Country = country;
	m_Organisation = organisation;
	m_Latitude = latitude;
	m_Longitude = longitude;
	return true;
}

bool GeolocationData::LoadFromJSON(const json& geolocationDataJson)
{
	m_City = geolocationDataJson["city"].get<std::string>();
	m_Region = geolocationDataJson["region"].get<std::string>();
	m_Country = geolocationDataJson["country"].get<std::string>();
	m_Organisation = geolocationDataJson["org"].get<std::string>();

	std::string location = geolocationDataJson["loc"];
	size_t locationSeparator = location.find_first_of(',');
	if (locationSeparator == std::string::npos)
	{
		return false;
	}
	else
	{
		std::string latitude = location.substr(0, locationSeparator);
		std::string longitude = location.substr(locationSeparator + 1);
		m_Latitude = static_cast<float>(atof(latitude.c_str()));
		m_Longitude = static_cast<float>(atof(longitude.c_str()));
		return true;
	}
}

void GeolocationData::SaveToDatabase(Database* pDatabase)
{
	PreparedStatement addGeolocationStatement(pDatabase, "INSERT OR REPLACE INTO Geolocation VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7);");
	addGeolocationStatement.Bind(1, m_Address.ToString());
	addGeolocationStatement.Bind(2, m_City);
	addGeolocationStatement.Bind(3, m_Region);
	addGeolocationStatement.Bind(4, m_Country);
	addGeolocationStatement.Bind(5, m_Organisation);
	addGeolocationStatement.Bind(6, static_cast<double>(m_Latitude));
	addGeolocationStatement.Bind(7, static_cast<double>(m_Longitude));
	pDatabase->Execute(addGeolocationStatement);

	PreparedStatement updateCameraStatement(pDatabase, "UPDATE Cameras SET Geolocated=1 WHERE IP=?1;");
	updateCameraStatement.Bind(1, m_Address.ToString());
	pDatabase->Execute(updateCameraStatement);
}

}