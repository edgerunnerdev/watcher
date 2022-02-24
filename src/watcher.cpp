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

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>

#include <curl/curl.h>
#include <SDL.h>

#include "codecs/codecmanager.h"
#include "ext/json.h"
#include "imgui/imgui.h"
#include "tasks/googlesearch/googlesearch.h"
#include "tasks/task.h"
#include "configuration.h"
#include "geolocationdata.h"
#include "log.h"
#include "watcher.h"
#include "watcherrep.h"
#include "textureloader.h"

extern IMGUI_API ImGuiContext* GImGui;

namespace Watcher
{

Watcher* g_pWatcher = nullptr;

Watcher::Watcher(SDL_Window* pWindow, unsigned int scannerCount) :
m_Searching(false),
m_Active(true),
m_pDatabase(nullptr)
{
	g_pWatcher = this;

	Log::AddLogTarget(std::make_shared<FileLogger>("log.txt"));
#ifdef _WIN32
	Log::AddLogTarget(std::make_shared<VisualStudioLogger>());
#endif

	TextureLoader::Initialise();

	m_pConfiguration = std::make_unique<Configuration>();
	m_pRep = std::make_unique<WatcherRep>(pWindow);
    m_pCodecManager = std::make_unique<CodecManager>();

	InitialiseDatabase();

	// All the geolocation data needs to be loaded before the cameras are, as every 
	// camera will try to associate its IP address with a geolocation entry.
	InitialiseGeolocation();
	InitialiseCameras();
    InitialiseTasks();
}

Watcher::~Watcher()
{
	m_Tasks.clear();
	m_Active = false;
}

void Watcher::InitialiseDatabase()
{
    DatabaseUniquePtr pDatabase = std::make_unique<Database>();
    if (pDatabase->Initialise())
    {
        m_pDatabase = std::move(pDatabase);
    }
	else
	{
		Log::Error("Failed to initialise database.");
	}
}

void Watcher::GeolocationRequestCallback(const QueryResult& result, void* pData)
{

}

void Watcher::InitialiseGeolocation()
{
	PreparedStatement statement(m_pDatabase.get(), "SELECT * FROM Geolocation", &Watcher::LoadGeolocationDataCallback);
	m_pDatabase->Execute(statement);

	PreparedStatement query(m_pDatabase.get(), "SELECT IP FROM Cameras WHERE GeolocationId IS NULL", &Watcher::GeolocationRequestCallback);
	m_pDatabase->Execute(query);
}

void Watcher::InitialiseCameras()
{
	PreparedStatement query(m_pDatabase.get(), "SELECT * FROM Cameras", &Watcher::LoadCamerasCallback);
	m_pDatabase->Execute(query);
}

void Watcher::InitialiseTasks()
{
    m_Tasks.emplace_back(std::make_unique<Task>("Geolocation"));
    m_Tasks.emplace_back(std::make_unique<Tasks::GoogleSearch>());
    m_Tasks.emplace_back(std::make_unique<Task>("HTTP camera detector"));

	if (m_pDatabase->IsFreshlyCreated())
	{
		for (auto&& pTask : m_Tasks)
		{
			pTask->OnDatabaseCreated(m_pDatabase.get());
		}
	}
}

void Watcher::ProcessEvent(const SDL_Event& event)
{
	m_pRep->ProcessEvent(event);
}

void Watcher::Update()
{
	TextureLoader::Update();

	const float delta = ImGui::GetIO().DeltaTime;

	for (auto& pTask : m_Tasks)
	{
		pTask->Update(delta);
	}

	m_pRep->Update(delta);
	m_pRep->Render();
}

Task* Watcher::GetTask(const std::string& name) const
{
	for (auto&& pTask : m_Tasks)
	{
		if (pTask->GetName() == name)
		{
			return pTask.get();
		}
	}

	return nullptr;
}

void Watcher::OnMessageReceived(const json& message)
{
	const std::string& messageType = message["type"];
	if (messageType == "log")
	{
		const std::string& messageLevel = message["level"];
		const std::string& messagePlugin = message["plugin"];
		const std::string& messageText = message["message"];
		if (messageLevel == "warning") Log::Warning("%s %s", messagePlugin.c_str(), messageText.c_str());
		else if (messageLevel == "error") Log::Error("%s %s", messagePlugin.c_str(), messageText.c_str());
		else Log::Info("%s %s", messagePlugin.c_str(), messageText.c_str());
	}
	else if (messageType == "geolocation_result")
	{
		AddGeolocationData(message);
	}
	else if (messageType == "http_server_scan_result")
	{
		AddCamera(message);
	}
	else if (messageType == "stream_started")
	{
		CameraSharedPtr pCamera = FindCamera(message["url"]);
		if (pCamera != nullptr)
		{
			ChangeCameraState(pCamera, Camera::State::StreamAvailable);
		}
	}
}

CameraSharedPtr Watcher::FindCamera(const std::string& url)
{
	std::scoped_lock lock(m_CamerasMutex);
	for (CameraSharedPtr& pCamera : m_Cameras)
	{
		if (pCamera->GetURL() == url)
		{
			return pCamera;
		}
	}
	return CameraSharedPtr();
}

void Watcher::ChangeCameraState(CameraSharedPtr pCamera, Camera::State state)
{
	pCamera->SetState(state);

	PreparedStatement statement(m_pDatabase.get(), "UPDATE Cameras SET Type=?1, Date=?2 WHERE URL=?3;");
	statement.Bind(1, static_cast<int>(state));
	statement.Bind(2, GetDate());
	statement.Bind(3, pCamera->GetURL());
	m_pDatabase->Execute(statement);
}

void Watcher::LoadGeolocationDataCallback(const QueryResult& result, void* pData)
{
	for (auto& row : result.Get())
	{
		static const int numCells = 7;
		if (numCells != row.size())
		{
			Log::Error("Invalid number of rows returned from query in LoadGeolocationDataCallback(). Expected %d, got %d.", numCells, row.size());
			continue;
		}

		std::scoped_lock lock(g_pWatcher->m_GeolocationDataMutex);
		IPAddress address(row[0]->GetString());
		std::string city = row[1]->GetString();
		std::string region = row[2]->GetString();
		std::string country = row[3]->GetString();
		std::string organisation = row[4]->GetString();
		float latitude = static_cast<float>(row[5]->GetDouble());
		float longitude = static_cast<float>(row[6]->GetDouble());
		GeolocationDataSharedPtr pGeolocationData = std::make_shared<GeolocationData>(address);
		pGeolocationData->LoadFromDatabase(city, region, country, organisation, latitude, longitude);
		g_pWatcher->m_GeolocationData[address.ToString()] = pGeolocationData; // No need to lock map here as only the main thread is working on it at this point.
	}
}

void Watcher::LoadCamerasCallback(const QueryResult& result, void* pData)
{
	for (auto& row : result.Get())
	{
		static const int numCells = 7;
		if (numCells != row.size())
		{
			Log::Error("Invalid number of rows returned from query in LoadCamerasCallback(). Expected %d, got %d.", numCells, row.size());
			continue;
		}

		std::scoped_lock lock(g_pWatcher->m_CamerasMutex, g_pWatcher->m_GeolocationDataMutex);
		std::string ip(row[1]->GetString());
		IPAddress address(ip);
		// address.SetPort(row[2]->GetInt()); // TODO: pass port directly into camera constructor?

		CameraSharedPtr camera = std::make_shared<Camera>(row[3]->GetString(), row[0]->GetString(), address, Camera::State::Unknown);
		if (g_pWatcher->m_GeolocationData.find(ip) != g_pWatcher->m_GeolocationData.cend())
		{
			camera->SetGeolocationData(g_pWatcher->m_GeolocationData[ip]);
		}

		camera->SetState(static_cast<Camera::State>(row[6]->GetInt()));

		g_pWatcher->m_Cameras.push_back(camera);
	}
}

void Watcher::AddGeolocationData(const json& message)
{
	std::string addressStr = message["address"];
	const IPAddress address(addressStr);
	GeolocationDataSharedPtr pGeolocationData = std::make_shared<GeolocationData>(address);
	pGeolocationData->LoadFromJSON(message);
	Log::Info("Added geolocation data for %s: %s, %s", addressStr.c_str(), pGeolocationData->GetCity().c_str(), pGeolocationData->GetCountry().c_str());

	{
		std::scoped_lock lock(m_GeolocationDataMutex, m_CamerasMutex);
		m_GeolocationData[addressStr] = pGeolocationData;

		for (CameraSharedPtr& camera : m_Cameras)
		{
			if (camera->GetAddress().ToString() == pGeolocationData->GetIPAddress().ToString())
			{
				camera->SetGeolocationData(pGeolocationData);
			}
		}
	}

	pGeolocationData->SaveToDatabase(m_pDatabase.get());
}

// Returns the current timestamp in a format which can be understood by sqlite as a date (YYYY-MM-DD HH:MM:SS.SSS).
// See https://www.sqlitetutorial.net/sqlite-date/
std::string Watcher::GetDate() const
{
	time_t rawTime;
	static char buffer[128];
	time(&rawTime);

#ifdef _WIN32
	struct tm timeInfo;
	if (localtime_s(&timeInfo, &rawTime) == 0)
	{
		strftime(buffer, sizeof(buffer), "%F %T.000", &timeInfo);
		return std::string(buffer);
	}
	else
	{
		return std::string();
	}
#else
	struct tm* pTimeInfo = localtime(&rawTime);
	strftime(buffer, sizeof(buffer), "%F %T.000", pTimeInfo);
	return std::string(buffer);	
#endif
}

void Watcher::AddCamera(const json& message)
{
	if (message["type"] != "http_server_scan_result")
	{
		return;
	}

	if (message["is_camera"])
	{
		const std::string url = message["url"];
		const std::string ipAddress = message["ip_address"];
		int port = message["port"];
		const std::string title = message["title"];
		const std::string username;
		const std::string password;

		PreparedStatement addCameraStatement(m_pDatabase.get(), "INSERT OR REPLACE INTO Cameras VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7);");
		addCameraStatement.Bind(1, url);
		addCameraStatement.Bind(2, ipAddress);
		addCameraStatement.Bind(3, port);
		addCameraStatement.Bind(4, title);
		addCameraStatement.Bind(5, 0); // Geolocation pending.
		addCameraStatement.Bind(6, GetDate());
		addCameraStatement.Bind(7, static_cast<int>(Camera::State::Unknown));

		m_pDatabase->Execute(addCameraStatement);

		{
			std::scoped_lock lock(m_CamerasMutex);
			IPAddress fullAddress(ipAddress);
			// fullAddress.SetPort(port); // TODO: Set port directly in Camera's constructor?

			CameraSharedPtr camera = std::make_shared<Camera>(title, url, fullAddress);
			m_Cameras.push_back(camera);
		}
	}
}

void Watcher::SetSearching(bool state)
{
	m_Searching = state;

	for (auto& pTask : m_Tasks)
	{
		if (m_Searching)
		{
			pTask->Start();
		}
		else
		{
			pTask->Stop();
		}
	}
}

} // namespace Watcher