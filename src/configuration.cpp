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

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <shlobj.h>
#undef WIN32_LEAN_AND_MEAN
#else
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#endif

#include <fstream>
#include <string>
#include <SDL.h>
#include "ext/json.h"
#include "configuration.h"

namespace Watcher
{

static const std::string sGoogleCSEApiKey("google_cse_api_key");
static const std::string sGoogleCSEId("google_cse_id");

Configuration::Configuration() :
m_GoogleCSEApiKey("AIzaSyBHnMEOSkTM7Lazvou27FXgJb6M4hjn9uE"),
m_GoogleCSEId("016794710981670214596:sgntarey42m")
{
	CreateStorage();
	Load();
}

Configuration::~Configuration()
{
	Save();
}

void Configuration::CreateStorage()
{
#ifdef _WIN32
	// Return %USERPROFILE%\Saved Games for Windows Vista or newer
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb762188%28v=vs.85%29.aspx
	PWSTR pKnownFolderPath = nullptr;
	HRESULT result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pKnownFolderPath);
	SDL_assert_release(result == S_OK);
	std::wstring folder(pKnownFolderPath);
	CoTaskMemFree(pKnownFolderPath);
	m_StoragePath = folder;
	m_StoragePath.append("watcher");
	std::filesystem::create_directories(m_StoragePath);
#else
	struct passwd* pw = getpwuid(getuid());
	const char* pHomeDir = pw->pw_dir;
	m_StoragePath = std::filesystem::path(pHomeDir) / ".local" / "share" / "watcher";
#endif
}

void Configuration::Save()
{
	using json = nlohmann::json;
	json config;
	config[sGoogleCSEApiKey] = m_GoogleCSEApiKey;
	config[sGoogleCSEId] = m_GoogleCSEId;

	std::filesystem::path filePath = m_StoragePath / "config.json";
	std::ofstream file( filePath );
	file << config;
	file.close();
}

void Configuration::Load()
{
	using json = nlohmann::json;
	std::ifstream file( "config.json" );
	if ( file.is_open() )
	{
		json config;
		file >> config;
		file.close();

		json jGoogleCSEApiKey = config[sGoogleCSEApiKey];
		if (jGoogleCSEApiKey.is_string())
		{
			m_GoogleCSEApiKey = jGoogleCSEApiKey.get<std::string>();
		}

		json jGoogleCSEId = config[sGoogleCSEId];
		if (jGoogleCSEId.is_string())
		{
			m_GoogleCSEId = jGoogleCSEId.get<std::string>();
		}
	}
}

} // namespace Watcher
