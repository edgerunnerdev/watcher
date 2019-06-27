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

#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include <SDL.h>

#include "database/database.h"
#include "network/network.h"
#include "geo_info.h"

#include "json.h"
using json = nlohmann::json;

class Configuration;
class PluginManager;
class WatcherRep;
struct SDL_Window;

using ThreadVector = std::vector< std::thread >;
using IPVector = std::vector< Network::IPAddress >;
using WatcherRepUniquePtr = std::unique_ptr< WatcherRep >;
using GeoInfoVector = std::vector< GeoInfo >;
using ConfigurationUniquePtr = std::unique_ptr< Configuration >;
using PluginManagerUniquePtr = std::unique_ptr< PluginManager >;


class Watcher
{
public:
	Watcher( SDL_Window* pWindow, unsigned int scannerCount );
	~Watcher();
	void ProcessEvent( const SDL_Event& event );
	void Update();
	bool IsActive() const;
	Configuration* GetConfiguration() const;

	void OnWebServerFound( Network::IPAddress address );
	void OnMessageReceived( const json& message );

	GeoInfoVector GetGeoInfos();

private:
	static void GeolocationRequestCallback( const Database::QueryResult& result, void* pData );
	static void LoadGeoInfosCallback( const Database::QueryResult& result, void* pData );

	void RestartCameraDetection();
	void InitialiseDatabase();
	void InitialiseGeolocation();
	void LoadGeoInfos();
	void AddGeoInfo(const json& message);
	void AddCamera(const json& message);

	bool m_Active;
	Database::DatabaseUniquePtr m_pDatabase;

	std::mutex m_GeoInfoMutex;
	GeoInfoVector m_GeoInfos;

	WatcherRepUniquePtr m_pRep;
	ConfigurationUniquePtr m_pConfiguration;

	PluginManagerUniquePtr m_pPluginManager;
};

extern Watcher* g_pWatcher;

inline bool Watcher::IsActive() const
{
	return m_Active;
}

inline Configuration* Watcher::GetConfiguration() const
{
	return m_pConfiguration.get();
}

inline GeoInfoVector Watcher::GetGeoInfos()
{
	std::lock_guard< std::mutex > lock( m_GeoInfoMutex );
	GeoInfoVector v = m_GeoInfos;
	return v;
}
