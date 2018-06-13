#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <SDL.h>
#include "imgui/imgui.h"
#include "sqlite/sqlite3.h"
#include "camera_scanner.h"
#include "configuration.h"
#include "database_helpers.h"
#include "ip_generator.h"
#include "watcher.h"
#include "scanner.h"
#include "watcher_rep.h"

Watcher* g_pWatcher = nullptr;

Watcher::Watcher( SDL_Window* pWindow, unsigned int scannerCount ) :
m_Active( true ),
m_WebServerScannerMode( WebServerScannerMode::None ),
m_pDatabase( nullptr )
{
	g_pWatcher = this;
	m_MainThreadID = std::this_thread::get_id();
	m_pConfiguration = std::make_unique< Configuration >();

	m_pRep = std::make_unique< WatcherRep >( pWindow );
	m_pIPGenerator = std::make_unique< IPGenerator >( m_pConfiguration->GetWebScannerStartAddress() );
	sqlite3_open( "0x00-watcher.db", &m_pDatabase );

	PopulateCameraDetectionQueue();
	InitialiseGeoScanner();
	InitialiseCameraScanner();
}

Watcher::~Watcher()
{
	m_Active = false;
	for ( auto& thread : m_ScannerThreads )
	{
		thread.join();
	}

	m_CameraScannerThread.join();
	m_GeoScannerThread.join();

	sqlite3_close( m_pDatabase );

	m_pConfiguration->SetWebScannerStartAddress( m_pIPGenerator->GetCurrent() );
}

void Watcher::InitialiseWebServerScanners( unsigned int scannerCount )
{
	auto scannerThreadMain = []( Watcher* pWatcher, Scanner* pScanner )
	{
		std::vector< unsigned short > ports = { 80, 81, 82, 83, 84, 8080 };
		while ( pWatcher->IsActive() )
		{
			Network::IPAddress address = pWatcher->GetIPGenerator()->GetNext();
			
			for ( unsigned short port : ports )
			{
				address.SetPort( port );
				if ( pScanner->Scan( address ) )
				{
					pWatcher->OnWebServerFound( address );
				}
			}
		}
	};
	
	for ( unsigned int i = 0u; i < scannerCount; i++ )
	{
		ScannerUniquePtr pScanner = std::make_unique< Scanner >();
		m_ScannerThreads.emplace_back( scannerThreadMain, this, pScanner.get() );
		m_Scanners.push_back( std::move( pScanner ) );
	}
}

void Watcher::InitialiseCameraScanner()
{
	auto scannerThreadMain = []( Watcher* pWatcher, CameraScanner* pScanner )
	{
		while ( pWatcher->IsActive() )
		{
			Network::IPAddress address;
			if ( pWatcher->ConsumeCameraScannerQueue( address ) )
			{ 
				CameraScanResult scanResult = pScanner->Scan( address );
				pWatcher->OnCameraScanned( scanResult );
			}
			else
			{
				std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
			}
		}
	};
	
	m_pCameraScanner = std::make_unique< CameraScanner >();
	m_CameraScannerThread = std::thread( scannerThreadMain, this, m_pCameraScanner.get() );
}

void Watcher::InitialiseGeoScanner()
{
	auto scannerThreadMain = []( Watcher* pWatcher, GeoScanner* pScanner )
	{
		while ( pWatcher->IsActive() )
		{
			pScanner->Update();
		}
	};
	
	m_pGeoScanner = std::make_unique< GeoScanner >();
	m_GeoScannerThread = std::thread( scannerThreadMain, this, m_pGeoScanner.get() );
}

void Watcher::Update()
{
	ProcessDatabaseQueryQueue();

	m_pRep->Render();

	ImGui::SetNextWindowPos( ImVec2( 0, 0 ) );
	ImGui::SetNextWindowSize( ImVec2( 400, 0 ) );
	ImGui::Begin("LeftBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar );

	if ( ImGui::CollapsingHeader( "Webserver scanner", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		if ( m_WebServerScannerMode == WebServerScannerMode::None )
		{
			if ( ImGui::Button( "Begin scan (basic)" ) )
			{
				InitialiseWebServerScanners( 64 );
				m_WebServerScannerMode = WebServerScannerMode::Basic;
			}
			else if ( ImGui::Button( "Begin scan (zmap)" ) )
			{
				m_WebServerScannerMode = WebServerScannerMode::Zmap;
			}
		}
		else if ( m_WebServerScannerMode == WebServerScannerMode::Basic )
		{
			std::stringstream wss;
			unsigned int currentIP = m_pIPGenerator->GetCurrent().GetHost();
			unsigned int maxIP = ~0u;
			double ratio = static_cast< double >( currentIP ) / static_cast< double >( maxIP );
			float percent = static_cast< float >( ratio * 100.0f );
			//wss.precision( 3 );
			wss <<  "Address space scanned: " << percent << "%%";
			ImGui::Text( wss.str().c_str() );

			{
				std::stringstream ss;
				ss << "Most recent probe: " << m_pIPGenerator->GetCurrent().ToString();
				ImGui::Text( ss.str().c_str() );
			}


			if ( ImGui::TreeNode( "Active threads" ) )
			{
				ImGui::Columns( 2 );
				ImGui::SetColumnWidth( 0, 32 );
				unsigned int numScanners = m_Scanners.size();
				for ( unsigned int i = 0; i < numScanners; i++ )
				{
					std::stringstream ss;
					ss << "#" << ( i + 1 );
					ImGui::Text( ss.str().c_str() );
					ImGui::NextColumn();
					ImGui::Text( m_Scanners[ i ]->GetStatusText().c_str() );
					ImGui::NextColumn();
				}
				ImGui::Columns( 1 );

				ImGui::TreePop();
			}
		}
	}

	if ( ImGui::CollapsingHeader( "Camera scanner", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		std::stringstream queueSizeSS;
		queueSizeSS << "Queue size: " << m_CameraScannerQueue.size();
		ImGui::Text( queueSizeSS.str().c_str() );

		std::stringstream rulesSS;
		rulesSS << "Rules loaded: " << m_pCameraScanner->GetRuleCount();
		ImGui::Text( rulesSS.str().c_str() );

		if ( ImGui::TreeNode( "Results (100 most recent)" ) )
		{
			std::lock_guard< std::mutex > lock( m_CameraScanResultsMutex );
			ImGui::Columns( 3 );
			ImGui::SetColumnWidth( 0, 32 );
			for ( auto& scanResult : m_CameraScanResults )
			{
				ImGui::Text( scanResult.isCamera ? "x" : " " );
				ImGui::NextColumn();
				ImGui::Text( scanResult.address.ToString().c_str() );
				ImGui::NextColumn();
				ImGui::Text( scanResult.title.c_str() );
				ImGui::NextColumn();
			}
			ImGui::Columns( 1 );

			ImGui::TreePop();
		}
	}

	if ( ImGui::CollapsingHeader( "Geolocation", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		ImGui::Text( "Provider: ipinfo.io" );
		std::stringstream ss;
		ss << "Queue size: " << m_pGeoScanner->GetQueueSize();
		ImGui::Text( ss.str().c_str() );
	}

	if ( ImGui::CollapsingHeader( "Database operations" ) )
	{
		if ( ImGui::Button( "Restart camera scan" ) )
		{
			RestartCameraDetection();
		}
	}

	ImGui::End();

	ImGui::BeginTooltip();
	ImGui::Text("test tooltip");
    ImGui::EndTooltip();
}

void Watcher::OnWebServerFound( Network::IPAddress address )
{
	// Store this web server into the database, mark it as not parsed.
	std::stringstream ss;
	ss << "INSERT OR REPLACE INTO WebServers VALUES('" << address.GetHostAsString() << "', " << address.GetPort() << ", 0);";
	ExecuteDatabaseQuery( ss.str() );

	std::lock_guard< std::mutex > lock( m_CameraScannerQueueMutex );
	m_CameraScannerQueue.push_back( address );
}

void Watcher::RestartCameraDetection()
{
	ExecuteDatabaseQuery( "UPDATE WebServers SET Scanned=0;" );
}

// Loads from the database any IP addresses for web servers which have been
// found but not processed yet and adds them to the camera detection queue.
// Any servers which are identified at run time are added to the queue via the
// OnWebServerFound() callback instead.
void Watcher::PopulateCameraDetectionQueue()
{
	auto callback = []( void* notUsed, int argc, char** argv, char** azColName )
	{
		SDL_assert( argc == 2 );
		Network::IPAddress ipAddress( argv[0] );
		ipAddress.SetPort( atoi( argv[1] ) );
		g_pWatcher->OnWebServerAddedFromDatabase( ipAddress );
		return 0;
	};

	std::string query = "SELECT IP, Port FROM WebServers WHERE Scanned=0";
	char* pError = nullptr;
	int rc = sqlite3_exec( m_pDatabase, query.c_str(), callback, 0, &pError );
	if( rc != SQLITE_OK )
	{
		fprintf( stderr, "SQL error: %s\n", pError );
		sqlite3_free( pError );
	}
}

void Watcher::OnWebServerAddedFromDatabase( Network::IPAddress address )
{
	std::lock_guard< std::mutex > lock( m_CameraScannerQueueMutex );
	m_CameraScannerQueue.push_back( address );
}

void Watcher::OnCameraScanned( const CameraScanResult& result )
{
	std::lock_guard< std::mutex > lock( m_CameraScanResultsMutex );
	m_CameraScanResults.push_front( result );
	if ( m_CameraScanResults.size() > 100 )
	{
		m_CameraScanResults.pop_back();
	}

	std::stringstream wss;
	wss << "UPDATE WebServers SET Scanned=1 WHERE IP='" << result.address.GetHostAsString() << "';";
	ExecuteDatabaseQuery( wss.str() );

	if ( result.isCamera )
	{
		std::stringstream css;
		css << "INSERT OR REPLACE INTO Cameras VALUES('" 
			<< result.address.GetHostAsString() << "', " // Host
			<< result.address.GetPort()	// Port
			<< ", 0, '" // Type (unused at the moment)
			<< result.title // Parsed title
			<< "', 0);"; // Geolocation pending
		ExecuteDatabaseQuery( css.str() );
	}
}

void Watcher::ExecuteDatabaseQuery( const std::string& query )
{
	if ( std::this_thread::get_id() != m_MainThreadID )
	{
		std::lock_guard< std::mutex > lock( m_QueryQueueMutex );
		m_QueryQueue.push_back( query );
	}
	else
	{
		::ExecuteDatabaseQuery( m_pDatabase, query.c_str() );
	}
}

void Watcher::ProcessDatabaseQueryQueue()
{
	std::lock_guard< std::mutex > lock( m_QueryQueueMutex );
	for ( std::string& query : m_QueryQueue )
	{
		ExecuteDatabaseQuery( query );
	}
	m_QueryQueue.clear();
}

// Takes an address from the camera scanner queue.
// Returns false if the queue is empty.
bool Watcher::ConsumeCameraScannerQueue( Network::IPAddress& address )
{
	std::lock_guard< std::mutex > lock( m_CameraScannerQueueMutex );
	if ( m_CameraScannerQueue.empty() )
	{
		return false;
	}
	else
	{
		address = m_CameraScannerQueue.back();
		m_CameraScannerQueue.pop_back();
		return true;
	}
}

void Watcher::OnGeoInfoAdded( const GeoInfo& geoInfo )
{
	std::lock_guard< std::mutex > lock( m_GeoInfoMutex );
	m_GeoInfos.push_back( geoInfo );
}