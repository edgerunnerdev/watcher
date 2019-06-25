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

Watcher::Watcher( SDL_Window* pWindow, unsigned int scannerCount ) :
m_Active( true ),
m_pDatabase( nullptr )
{
	g_pWatcher = this;

	Log::AddLogTarget( std::make_shared< FileLogger >( "log.txt" ) );
#ifdef _WIN32
	Log::AddLogTarget( std::make_shared< VisualStudioLogger >() );
#endif

	TextureLoader::Initialise();

	m_pConfiguration = std::make_unique< Configuration >();
	m_pDatabase = std::make_unique< Database::Database >( "watcher.db" );

	m_pRep = std::make_unique< WatcherRep >( pWindow );

	m_pPluginManager = std::make_unique< PluginManager >();
	InitialiseGeolocation();
}

Watcher::~Watcher()
{
	m_Active = false;
}

void Watcher::GeolocationRequestCallback( const Database::QueryResult& result, void* pData )
{
	PluginManager* pPluginManager = reinterpret_cast< PluginManager* >( pData );
	for ( auto& row : result.Get() )
	{
		for ( auto& cell : row )
		{
			json message = 
			{
				{ "type", "geolocation_request" },
				{ "address", cell->GetString() },
			};
			pPluginManager->BroadcastMessage( message );
		}
	}
}

void Watcher::InitialiseGeolocation()
{
	LoadGeoInfos();

	Database::PreparedStatement query( m_pDatabase.get(), "SELECT IP FROM Cameras WHERE Geo=0", &Watcher::GeolocationRequestCallback, m_pPluginManager.get() );
	m_pDatabase->Execute( query );
}

void Watcher::ProcessEvent( const SDL_Event& event ) 
{
	m_pRep->ProcessEvent( event );
}

void Watcher::Update()
{
	TextureLoader::Update();

	m_pRep->Update();
	m_pRep->Render();

	ImGui::SetNextWindowPos( ImVec2( 0, 0 ) );
	ImGui::SetNextWindowSize( ImVec2( 400, 0 ) );
	ImGui::Begin("LeftBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar );

	for ( Plugin* pPlugin : m_pPluginManager->GetPlugins() )
	{
		pPlugin->DrawUI( GImGui );
	}

	if ( ImGui::CollapsingHeader( "Plugins", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		if ( m_pPluginManager->GetPlugins().empty() )
		{
			ImGui::Text( "No plugins found." );
		}
		else
		{
			ImGui::Columns( 2 );
			ImGui::Text( "Plugin" ); ImGui::NextColumn();
			ImGui::Text( "Version" ); ImGui::NextColumn();
			for ( Plugin* pPlugin : m_pPluginManager->GetPlugins() )
			{
				ImGui::Text( pPlugin->GetName().c_str() );
				ImGui::NextColumn();

				int version[ 3 ] = { 0, 0, 0 };
				pPlugin->GetVersion( version[ 0 ], version[ 1 ], version[ 2 ] );
				ImGui::Text( "%d.%d.%d", version[ 0 ], version[ 1 ], version[ 2 ] );
				ImGui::NextColumn();
			}
			ImGui::Columns( 1 );
		}
	}

	ImGui::End();
}

void Watcher::OnMessageReceived( const json& message )
{
	const std::string& messageType = message[ "type" ];
	if ( messageType == "log" )
	{
		const std::string& messageLevel = message[ "level" ];
		const std::string& messagePlugin = message[ "plugin" ];
		const std::string& messageText = message[ "message" ];
		if ( messageLevel == "warning" ) Log::Warning( "%s %s", messagePlugin.c_str(), messageText.c_str() );
		else if ( messageLevel == "error" ) Log::Error( "%s %s", messagePlugin.c_str(), messageText.c_str() );
		else Log::Info( "%s %s", messagePlugin.c_str(), messageText.c_str() );
	}
	else if ( messageType == "geolocation_result" )
	{
		AddGeoInfo( message );
	}
	else
	{
		m_pPluginManager->BroadcastMessage(message);
	}
}

void Watcher::OnWebServerFound( Network::IPAddress address )
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

//void Watcher::PopulateCameraDetectionQueueCallback( const Database::QueryResult& result, void* pData )
//{
//	for ( auto& row : result.Get() )
//	{
//		const int numCells = 2;
//		if ( row.size() != 2 )
//		{
//			Log::Error( "Invalid number of rows returned from query in PopulateCameraDetectionQueueCallback(). Expected %d, got %d.", numCells, row.size() );
//		}
//		else
//		{
//			Network::IPAddress ipAddress( row[ 0 ]->GetString() );
//			ipAddress.SetPort( row[ 1 ]->GetInt() );
//
//			std::lock_guard< std::mutex > lock( g_pWatcher->m_CameraScannerQueueMutex );
//			g_pWatcher->m_CameraScannerQueue.push_back( ipAddress );
//		}
//	}
//}

// Loads from the database any IP addresses for web servers which have been
// found but not processed yet and adds them to the camera detection queue.
// Any servers which are identified at run time are added to the queue via the
// OnWebServerFound() callback instead.
void Watcher::PopulateCameraDetectionQueue()
{
	//Database::PreparedStatement statement( m_pDatabase.get(), "SELECT IP, Port FROM WebServers WHERE Scanned=0", &Watcher::PopulateCameraDetectionQueueCallback );
	//m_pDatabase->Execute( statement );
}

//void Watcher::OnCameraScanned( const CameraScanResult& result )
//{
//	if ( result.title.empty() == false || result.isCamera )
//	{
//		{
//			std::lock_guard< std::mutex > lock( m_CameraScanResultsMutex );
//			m_CameraScanResults.push_front( result );
//			if ( m_CameraScanResults.size() > 100 )
//			{
//				m_CameraScanResults.pop_back();
//			}
//		}	
//	}
//
//	Database::PreparedStatement statement( m_pDatabase.get(), "UPDATE WebServers SET Scanned=1 WHERE IP=?1;" );
//	statement.Bind( 1, result.address.GetHostAsString() );
//	m_pDatabase->Execute( statement );
//
//	if ( result.isCamera )
//	{
//		Database::PreparedStatement updateCameraStatement( m_pDatabase.get(), "UPDATE Cameras SET Geo=1 WHERE IP=?1;" );
//		updateCameraStatement.Bind( 1, result.address.ToString() );
//		m_pDatabase->Execute( updateCameraStatement );
//
//		Database::PreparedStatement addCameraStatement( m_pDatabase.get(), "INSERT OR REPLACE INTO Cameras VALUES(?1, ?2, ?3, ?4, ?5);" );
//		addCameraStatement.Bind( 1, result.address.GetHostAsString() );
//		addCameraStatement.Bind( 2, result.address.GetPort() );
//		addCameraStatement.Bind( 3, 0 ); // Type (unused at the moment).
//		addCameraStatement.Bind( 4, result.title );
//		addCameraStatement.Bind( 5, 0 ); // Geolocation pending.
//		m_pDatabase->Execute( addCameraStatement );
//
//		json message = 
//		{
//			{ "type", "geolocation_request" },
//			{ "address", result.address.ToString() },
//		};
//		m_pPluginManager->BroadcastMessage( message );
//	}
//	else
//	{
//		Log::Info( "No camera detected at %s, title: %s", result.address.ToString().c_str(), result.title.c_str() );
//	}
//}

void Watcher::LoadGeoInfosCallback( const Database::QueryResult& result, void* pData )
{
	for ( auto& row : result.Get() )
	{
		const int numCells = 7;
		if ( numCells != row.size() )
		{
			Log::Error( "Invalid number of rows returned from query in LoadGeoInfosCallback(). Expected %d, got %d.", numCells, row.size() );
			continue;
		}

		std::lock_guard< std::mutex > lock( g_pWatcher->m_GeoInfoMutex );
		Network::IPAddress address( row[ 0 ]->GetString() );
		std::string city = row[ 1 ]->GetString();
		std::string region = row[ 2 ]->GetString();
		std::string country = row[ 3 ]->GetString();
		std::string organisation = row[ 4 ]->GetString();
		float latitude = static_cast< float >( row[ 5 ]->GetDouble() );
		float longitude = static_cast< float >( row[ 6 ]->GetDouble() );
		GeoInfo geoInfo( address );
		geoInfo.LoadFromDatabase( city, region, country, organisation, latitude, longitude );
		g_pWatcher->m_GeoInfos.push_back( geoInfo );
	}
}

// Loads all geolocation information which had previously been stored in the database.
void Watcher::LoadGeoInfos()
{
	Database::PreparedStatement statement( m_pDatabase.get(), "SELECT * FROM Geolocation", &Watcher::LoadGeoInfosCallback );
	m_pDatabase->Execute( statement );
}

void Watcher::AddGeoInfo( const json& message )
{
	std::string addressStr = message[ "address" ];
	const Network::IPAddress address( addressStr );
	GeoInfo geoInfo( address );
	geoInfo.LoadFromJSON( message );
	Log::Info( "Added geo info for %s: %s, %s", addressStr.c_str(), geoInfo.GetCity().c_str(), geoInfo.GetCountry().c_str() );

	{
		std::lock_guard< std::mutex > lock( m_GeoInfoMutex );
		m_GeoInfos.push_back( geoInfo );
	}

	geoInfo.SaveToDatabase( m_pDatabase.get() );
}
