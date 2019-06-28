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

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <SDL.h>
#include "ext/json.h"
#include "imgui/imgui.h"
#include "configuration.h"
#include "geo_info.h"
#include "log.h"
#include "watcher.h"
#include "watcher_rep.h"
#include "plugin_manager.h"
#include "plugin.h"
#include "texture_loader.h"

Watcher* g_pWatcher = nullptr;
extern IMGUI_API ImGuiContext* GImGui;

Watcher::Watcher(SDL_Window* pWindow, unsigned int scannerCount) :
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
	m_pRep = std::make_unique< WatcherRep >(pWindow);

	m_pPluginManager = std::make_unique<PluginManager>();
	InitialiseDatabase();
	InitialiseGeolocation();
}

Watcher::~Watcher()
{
	m_Active = false;
}

// Initialises our database. If the database file didn't exist previously, we make a copy
// of the stub.db file, which contains the basic database structure.
// This approach is easier to edit than creating the database programatically.
void Watcher::InitialiseDatabase()
{
	const std::string databaseFilename("watcher.db");
	std::ifstream databaseFile(databaseFilename);
	if (databaseFile.good())
	{
		databaseFile.close();
	}
	else
	{
		std::ifstream source("stub.db", std::ios::binary);
		if (source.good() == false)
		{
			Log::Error("Couldn't find stub.db file.");
			return;
		}
		std::ofstream destination(databaseFilename, std::ios::binary);
		destination << source.rdbuf();
	}
	m_pDatabase = std::make_unique< Database::Database >(databaseFilename);
}

void Watcher::GeolocationRequestCallback(const Database::QueryResult& result, void* pData)
{
	PluginManager* pPluginManager = reinterpret_cast<PluginManager*>(pData);
	for (auto& row : result.Get())
	{
		for (auto& cell : row)
		{
			json message =
			{
				{ "type", "geolocation_request" },
				{ "ip_address", cell->GetString() },
			};
			pPluginManager->BroadcastMessage(message);
		}
	}
}

void Watcher::InitialiseGeolocation()
{
	LoadGeoInfos();

	Database::PreparedStatement query(m_pDatabase.get(), "SELECT IP FROM Cameras WHERE Geolocated=0", &Watcher::GeolocationRequestCallback, m_pPluginManager.get());
	m_pDatabase->Execute(query);
}

void Watcher::ProcessEvent(const SDL_Event& event)
{
	m_pRep->ProcessEvent(event);
}

void Watcher::Update()
{
	TextureLoader::Update();

	m_pRep->Update();
	m_pRep->Render();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(400, 0));
	ImGui::Begin("LeftBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);

	for (Plugin* pPlugin : m_pPluginManager->GetPlugins())
	{
		pPlugin->DrawUI(GImGui);
	}

	if (ImGui::CollapsingHeader("Plugins"))
	{
		if (m_pPluginManager->GetPlugins().empty())
		{
			ImGui::Text("No plugins found.");
		}
		else
		{
			ImGui::Columns(2);
			ImGui::Text("Plugin"); ImGui::NextColumn();
			ImGui::Text("Version"); ImGui::NextColumn();
			for (Plugin* pPlugin : m_pPluginManager->GetPlugins())
			{
				ImGui::Text(pPlugin->GetName().c_str());
				ImGui::NextColumn();

				int version[3] = { 0, 0, 0 };
				pPlugin->GetVersion(version[0], version[1], version[2]);
				ImGui::Text("%d.%d.%d", version[0], version[1], version[2]);
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
	}

	ImGui::End();
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
		AddGeoInfo(message);
	}
	else if (messageType == "http_server_scan_result")
	{
		AddCamera(message);
	}

	m_pPluginManager->BroadcastMessage(message);
}

void Watcher::OnWebServerFound(Network::IPAddress address)
{
	// Store this web server into the database, mark it as not parsed.
	//Database::PreparedStatement statement( m_pDatabase.get(), "INSERT OR REPLACE INTO WebServers VALUES(?1, ?2, 0);" );
	//statement.Bind( 1, address.GetHostAsString() );
	//statement.Bind( 2, address.GetPort() );
	//m_pDatabase->Execute( statement );

	//std::lock_guard< std::mutex > lock( m_CameraScannerQueueMutex );
	//m_CameraScannerQueue.push_back( address );
}

void Watcher::RestartCameraDetection()
{
	//Database::PreparedStatement statement( m_pDatabase.get(), "UPDATE WebServers SET Scanned=0;" );
	//m_pDatabase->Execute( statement );
}

void Watcher::LoadGeoInfosCallback(const Database::QueryResult& result, void* pData)
{
	for (auto& row : result.Get())
	{
		const int numCells = 7;
		if (numCells != row.size())
		{
			Log::Error("Invalid number of rows returned from query in LoadGeoInfosCallback(). Expected %d, got %d.", numCells, row.size());
			continue;
		}

		std::lock_guard< std::mutex > lock(g_pWatcher->m_GeoInfoMutex);
		Network::IPAddress address(row[0]->GetString());
		std::string city = row[1]->GetString();
		std::string region = row[2]->GetString();
		std::string country = row[3]->GetString();
		std::string organisation = row[4]->GetString();
		float latitude = static_cast<float>(row[5]->GetDouble());
		float longitude = static_cast<float>(row[6]->GetDouble());
		GeoInfo geoInfo(address);
		geoInfo.LoadFromDatabase(city, region, country, organisation, latitude, longitude);
		g_pWatcher->m_GeoInfos.push_back(geoInfo);
	}
}

// Loads all geolocation information which had previously been stored in the database.
void Watcher::LoadGeoInfos()
{
	Database::PreparedStatement statement(m_pDatabase.get(), "SELECT * FROM Geolocation", &Watcher::LoadGeoInfosCallback);
	m_pDatabase->Execute(statement);
}

void Watcher::AddGeoInfo(const json& message)
{
	std::string addressStr = message["address"];
	const Network::IPAddress address(addressStr);
	GeoInfo geoInfo(address);
	geoInfo.LoadFromJSON(message);
	Log::Info("Added geo info for %s: %s, %s", addressStr.c_str(), geoInfo.GetCity().c_str(), geoInfo.GetCountry().c_str());

	{
		std::lock_guard< std::mutex > lock(m_GeoInfoMutex);
		m_GeoInfos.push_back(geoInfo);
	}

	geoInfo.SaveToDatabase(m_pDatabase.get());
}

void Watcher::AddCamera(const json& message)
{
	if (message["type"] != "http_server_scan_result")
	{
		Log::Warning("Called AddCamera() with a message type of '%s', rather than the expected 'http_server_scan_result'", message["type"]);
		return;
	}

	if (message["is_camera"])
	{
		const std::string url = message["url"];
		const std::string ipAddress = message["ip_address"];
		const std::string title = message["title"];

		Database::PreparedStatement addCameraStatement(m_pDatabase.get(), "INSERT OR REPLACE INTO Cameras VALUES(?1, ?2, ?3, ?4);");
		addCameraStatement.Bind(1, url);
		addCameraStatement.Bind(2, ipAddress);
		addCameraStatement.Bind(3, title);
		addCameraStatement.Bind(4, 0); // Geolocation pending.
		m_pDatabase->Execute(addCameraStatement);

		json message =
		{
			{ "type", "geolocation_request" },
			{ "ip_address", ipAddress },
		};
		m_pPluginManager->BroadcastMessage(message);
	}
}
