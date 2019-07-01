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
#include <unordered_map>
#include <vector>

#include <SDL.h>

#include "database/database.h"
#include "network/network.h"
#include "camera.h"
#include "geolocationdata.h"

#include "json.h"
using json = nlohmann::json;

class Configuration;
class PluginManager;
class WatcherRep;
struct SDL_Window;

using WatcherRepUniquePtr = std::unique_ptr< WatcherRep >;
using GeolocationDataMap = std::unordered_map<std::string, GeolocationDataSharedPtr>;
using ConfigurationUniquePtr = std::unique_ptr< Configuration >;
using PluginManagerUniquePtr = std::unique_ptr< PluginManager >;


class Watcher
{
public:
	Watcher(SDL_Window* pWindow, unsigned int scannerCount);
	~Watcher();
	void ProcessEvent(const SDL_Event& event);
	void Update();
	bool IsActive() const;
	Configuration* GetConfiguration() const;

	void OnMessageReceived(const json& message);

	CameraVector GetCameras() const;

private:
	static void GeolocationRequestCallback(const Database::QueryResult& result, void* pData);
	static void LoadGeolocationDataCallback(const Database::QueryResult& result, void* pData);
	static void LoadCamerasCallback(const Database::QueryResult& result, void* pData);

	void InitialiseDatabase();
	void InitialiseGeolocation();
	void InitialiseCameras();
	void AddGeolocationData(const json& message);
	void AddCamera(const json& message);

	bool m_Active;
	Database::DatabaseUniquePtr m_pDatabase;

	std::mutex m_GeolocationDataMutex;
	GeolocationDataMap m_GeolocationData;

	mutable std::mutex m_CamerasMutex;
	CameraVector m_Cameras;

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

inline CameraVector Watcher::GetCameras() const
{
	std::scoped_lock lock(m_CamerasMutex);
	return m_Cameras;
}
