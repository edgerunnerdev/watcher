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

#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <SDL.h>

#include "database/database.h"
#include "camera.h"
#include "geolocationdata.h"

#include "json.h"
using json = nlohmann::json;

struct SDL_Window;

namespace Watcher
{

class CodecManager;
class Configuration;
class Task;
class WatcherRep;

using CodecManagerUniquePtr = std::unique_ptr<CodecManager>;
using WatcherRepUniquePtr = std::unique_ptr<WatcherRep>;
using GeolocationDataMap = std::unordered_map<std::string, GeolocationDataSharedPtr>;
using ConfigurationUniquePtr = std::unique_ptr<Configuration>;
using TaskUniquePtr = std::unique_ptr<Task>;
using TaskVector = std::vector<TaskUniquePtr>;

class Watcher
{
public:
	Watcher(SDL_Window* pWindow, unsigned int scannerCount);
	~Watcher();
	void ProcessEvent(const SDL_Event& event);
	void Update();
    bool IsSearching() const;
    void SetSearching(bool state);
	bool IsActive() const;
	Configuration* GetConfiguration() const;
    const TaskVector& GetTasks() const;
	Task* GetTask(const std::string& name) const;

	void OnMessageReceived(const json& message);

	CameraVector GetCameras() const;
    CodecManager* GetCodecManager() const;

	Database* GetDatabase() const;

private:
	static void GeolocationRequestCallback(const QueryResult& result, void* pData);
	static void LoadGeolocationDataCallback(const QueryResult& result, void* pData);
	static void LoadCamerasCallback(const QueryResult& result, void* pData);

	void InitialiseDatabase();
	void InitialiseGeolocation();
	void InitialiseCameras();
    void InitialiseTasks();
	void AddGeolocationData(const json& message);
	void AddCamera(const json& message);
	std::string GetDate() const;
	CameraSharedPtr FindCamera(const std::string& url);
	void ChangeCameraState(CameraSharedPtr pCamera, Camera::State state);

    bool m_Searching;
	bool m_Active;
	DatabaseUniquePtr m_pDatabase;

	std::mutex m_GeolocationDataMutex;
	GeolocationDataMap m_GeolocationData;

	mutable std::mutex m_CamerasMutex;
	CameraVector m_Cameras;

	WatcherRepUniquePtr m_pRep;
	ConfigurationUniquePtr m_pConfiguration;
    TaskVector m_Tasks;
    CodecManagerUniquePtr m_pCodecManager;
};

extern Watcher* g_pWatcher;

inline bool Watcher::IsSearching() const
{
    return m_Searching;
}

inline bool Watcher::IsActive() const
{
	return m_Active;
}

inline Configuration* Watcher::GetConfiguration() const
{
	return m_pConfiguration.get();
}

inline const TaskVector& Watcher::GetTasks() const
{
    return m_Tasks;
}

inline CameraVector Watcher::GetCameras() const
{
	std::scoped_lock lock(m_CamerasMutex);
	return m_Cameras;
}

inline CodecManager* Watcher::GetCodecManager() const
{
    return m_pCodecManager.get();
}

inline Database* Watcher::GetDatabase() const
{
	return m_pDatabase.get();
}

} // namespace Watcher
