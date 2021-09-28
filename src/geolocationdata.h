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

#pragma once

#include <memory>
#include "ipaddress.h"
#include "json.h"
using json = nlohmann::json;

namespace Watcher
{

class Database;
class GeolocationData;
using GeolocationDataSharedPtr = std::shared_ptr<GeolocationData>;

class GeolocationData
{
public:
	GeolocationData(IPAddress address);
	bool LoadFromDatabase(const std::string& city, const std::string& region, const std::string& country, const std::string& organisation, float x, float y);
	bool LoadFromJSON(const json& message);
	void SaveToDatabase(Database* pDatabase);

	const IPAddress& GetIPAddress() const;
	const std::string& GetCity() const;
	const std::string& GetRegion() const;
	const std::string& GetCountry() const;
	const std::string& GetOrganisation() const;
	float GetLatitude() const;
	float GetLongitude() const;

private:
	IPAddress m_Address;
	std::string m_City;
	std::string m_Region;
	std::string m_Country;
	std::string m_Organisation;
	float m_Latitude;
	float m_Longitude;
};

inline const IPAddress& GeolocationData::GetIPAddress() const
{
	return m_Address;
}

inline const std::string& GeolocationData::GetCity() const
{
	return m_City;
}

inline const std::string& GeolocationData::GetRegion() const
{
	return m_Region;
}

inline const std::string& GeolocationData::GetCountry() const
{
	return m_Country;
}

inline const std::string& GeolocationData::GetOrganisation() const
{
	return m_Organisation;
}

inline float GeolocationData::GetLatitude() const
{
	return m_Latitude;
}

inline float GeolocationData::GetLongitude() const
{
	return m_Longitude;
}

} // namespace Watcher
