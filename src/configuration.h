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

#include <filesystem>
#include <vector>

namespace Watcher
{

class Configuration
{
public:
	Configuration();
	~Configuration();

	std::filesystem::path GetStoragePath() const;
	const std::string& GetGoogleCSEApiKey() const;
	void SetGoogleCSEApiKey(const std::string& value);
	const std::string& GetGoogleCSEId() const;
	void SetGoogleCSEId(const std::string& value);

private:
	void CreateStorage();
	void Save();
	void Load();

	std::filesystem::path m_StoragePath;
	std::string m_GoogleCSEApiKey;
	std::string m_GoogleCSEId;
};

inline std::filesystem::path Configuration::GetStoragePath() const
{
	return m_StoragePath;
}

inline const std::string& Configuration::GetGoogleCSEApiKey() const
{
	return m_GoogleCSEApiKey;
}

inline void Configuration::SetGoogleCSEApiKey(const std::string& value)
{
	m_GoogleCSEApiKey = value;
}

inline const std::string& Configuration::GetGoogleCSEId() const
{
	return m_GoogleCSEId;
}

inline void Configuration::SetGoogleCSEId(const std::string& value)
{
	m_GoogleCSEId = value;
}

} // namespace Watcher
