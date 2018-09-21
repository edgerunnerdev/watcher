#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <SDL.h>
#include "imgui/imgui.h"
#include "sqlite/sqlite3.h"
#include "port_scanner/coverage.h"
#include "camera_scanner.h"
#include "configuration.h"
#include "database_helpers.h"
#include "ip_generator.h"
#include "watcher.h"
#include "watcher_rep.h"
#include "internet_scanner_basic.h"
#include "internet_scanner_nmap.h"
#include "internet_scanner_zmap.h"

Watcher* g_pWatcher = nullptr;

Watcher::Watcher( SDL_Window* pWindow, unsigned int scannerCount ) :
m_Active( true ),
m_WebServerScannerMode( InternetScannerMode::None ),
m_pDatabase( nullptr )
{
	g_pWatcher = this;
	m_pConfiguration = std::make_unique< Configuration >();

	m_pRep = std::make_unique< WatcherRep >( pWindow );
	m_pIPGenerator = std::make_unique< IPGenerator >( m_pConfiguration->GetWebScannerStartAddress() );
	sqlite3_open( "0x00-watcher.db", &m_pDatabase );
	sqlite3_busy_timeout( m_pDatabase, 1000 );

	m_pPortScannerCoverage = std::make_unique< PortScanner::Coverage >();

	PopulateCameraDetectionQueue();
	InitialiseGeoScanner();
	InitialiseCameraScanners( 8 );
}

Watcher::~Watcher()
{
	m_Active = false;
	for ( auto& thread : m_InternetScannerBasicThreads )
	{
		if ( thread.joinable() )
		{
			thread.join();
		}
	}

	for ( auto& thread : m_CameraScannerThreads )
	{
		if ( thread.joinable() )
		{
			thread.join();
		}
	}

	if ( m_GeoScannerThread.joinable() )
	{
		m_GeoScannerThread.join();
	}

	if ( m_InternetScannerNmapThread.joinable() )
	{
		m_InternetScannerNmapThread.join();
	}

	if ( m_InternetScannerZmapThread.joinable() )
	{
		m_InternetScannerZmapThread.join();
	}

	sqlite3_close( m_pDatabase );

	m_pConfiguration->SetWebScannerStartAddress( m_pIPGenerator->GetCurrent() );
}

void Watcher::InitialiseInternetScannerBasic( unsigned int scannerCount )
{
	auto threadMain = []( InternetScanner* pScanner )
	{
		const Network::PortVector& ports = g_pWatcher->GetConfiguration()->GetWebScannerPorts();
		while ( g_pWatcher->IsActive() )
		{
			Network::IPAddress address = g_pWatcher->GetIPGenerator()->GetNext();
			pScanner->Scan( address, ports );
		}
	};

	for ( unsigned int i = 0u; i < scannerCount; i++ )
	{
		InternetScannerBasicUniquePtr pScanner = std::make_unique< InternetScannerBasic >();
		m_InternetScannerBasicThreads.emplace_back( threadMain, pScanner.get() );
		m_InternetScannerBasic.push_back( std::move( pScanner ) );
	}
}

void sWebScannerThreadMain( InternetScanner* pScanner )
{
	const Network::PortVector& ports = g_pWatcher->GetConfiguration()->GetWebScannerPorts();
	Network::IPAddress address = g_pWatcher->GetIPGenerator()->GetCurrent();
	while ( g_pWatcher->IsActive() )
	{
		if ( pScanner->Scan( address, ports ) )
		{
			address = g_pWatcher->GetIPGenerator()->GetNext( 16 );
		}
	}
}

void Watcher::InitialiseNmap()
{
	m_pInternetScannerNmap = std::make_unique< InternetScannerNmap >();
	m_InternetScannerNmapThread = std::thread( sWebScannerThreadMain, m_pInternetScannerNmap.get() );
}

void Watcher::InitialiseZmap()
{
	m_pInternetScannerZmap = std::make_unique< InternetScannerZmap >();
	m_InternetScannerZmapThread = std::thread( sWebScannerThreadMain, m_pInternetScannerZmap.get() );
}

void Watcher::InitialiseCameraScanners( unsigned int scannerCount )
{
	auto threadMain = []( Watcher* pWatcher, CameraScanner* pScanner )
	{
		sqlite3* pDatabase;
		sqlite3_open( "0x00-watcher.db", &pDatabase );
		sqlite3_busy_timeout( pDatabase, 1000 );

		while ( pWatcher->IsActive() )
		{
			Network::IPAddress address;
			if ( pWatcher->ConsumeCameraScannerQueue( address ) )
			{ 
				CameraScanResult scanResult = pScanner->Scan( address );
				pWatcher->OnCameraScanned( pDatabase, scanResult );
			}
			else
			{
				std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
			}
		}

		sqlite3_close( pDatabase );
	};

	for ( unsigned int i = 0u; i < scannerCount; i++ )
	{
		CameraScannerUniquePtr pScanner = std::make_unique< CameraScanner >();
		m_CameraScannerThreads.emplace_back( threadMain, this, pScanner.get() );
		m_CameraScanners.push_back( std::move( pScanner ) );
	}
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
	m_pRep->Update();
	m_pRep->Render();

	m_pPortScannerCoverage->DrawUI();

	ImGui::SetNextWindowPos( ImVec2( 0, 0 ) );
	ImGui::SetNextWindowSize( ImVec2( 400, 0 ) );
	ImGui::Begin("LeftBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar );

	if ( ImGui::CollapsingHeader( "Webserver scanner", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		if ( m_WebServerScannerMode == InternetScannerMode::None )
		{
			if ( ImGui::Button( "Begin scan (basic)" ) )
			{
				m_WebServerScannerMode = InternetScannerMode::Basic;
				InitialiseInternetScannerBasic( 64 );
			}
			else if ( InternetScannerNmap::IsSupported() && ImGui::Button( "Begin scan (nmap)" ) )
			{
				m_WebServerScannerMode = InternetScannerMode::Nmap;
				InitialiseNmap();
			}
			else if ( InternetScannerZmap::IsSupported() && ImGui::Button( "Begin scan (zmap)" ) )
			{
				m_WebServerScannerMode = InternetScannerMode::Zmap;
				InitialiseZmap();
			}
		}
		else if ( m_WebServerScannerMode == InternetScannerMode::Basic )
		{
			std::stringstream wss;
			unsigned int currentIP = m_pIPGenerator->GetCurrent().GetHost();
			unsigned int maxIP = ~0u;
			double ratio = static_cast< double >( currentIP ) / static_cast< double >( maxIP );
			float percent = static_cast< float >( ratio * 100.0f );
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
				unsigned int numScanners = m_InternetScannerBasic.size();
				for ( unsigned int i = 0; i < numScanners; i++ )
				{
					std::stringstream ss;
					ss << "#" << ( i + 1 );
					ImGui::Text( ss.str().c_str() );
					ImGui::NextColumn();
					ImGui::Text( m_InternetScannerBasic[ i ]->GetStatusText().c_str() );
					ImGui::NextColumn();
				}
				ImGui::Columns( 1 );

				ImGui::TreePop();
			}
		}
		else if ( m_WebServerScannerMode == InternetScannerMode::Nmap )
		{
			ImGui::Text( m_pInternetScannerNmap->GetStatusText().c_str() );
		}
		else if ( m_WebServerScannerMode == InternetScannerMode::Zmap )
		{
			ImGui::Text( m_pInternetScannerZmap->GetStatusText().c_str() );
		}
	}

	if ( ImGui::CollapsingHeader( "Camera scanner", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		std::stringstream queueSizeSS;
		queueSizeSS << "Queue size: " << m_CameraScannerQueue.size();
		ImGui::Text( queueSizeSS.str().c_str() );

		//std::stringstream rulesSS;
		//rulesSS << "Rules loaded: " << m_pCameraScanner->GetRuleCount();
		//ImGui::Text( rulesSS.str().c_str() );

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
}

void Watcher::OnWebServerFound( sqlite3* pDatabase, Network::IPAddress address )
{
	if ( pDatabase != nullptr )
	{
		// Store this web server into the database, mark it as not parsed.
		std::stringstream ss;
		ss << "INSERT OR REPLACE INTO WebServers VALUES('" << address.GetHostAsString() << "', " << address.GetPort() << ", 0);";
		ExecuteDatabaseQuery( pDatabase, ss.str() );
	}

	std::lock_guard< std::mutex > lock( m_CameraScannerQueueMutex );
	m_CameraScannerQueue.push_back( address );
}

void Watcher::RestartCameraDetection()
{
	ExecuteDatabaseQuery( m_pDatabase, "UPDATE WebServers SET Scanned=0;" );
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
		g_pWatcher->OnWebServerFound( nullptr, ipAddress );
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

void Watcher::OnCameraScanned( sqlite3* pDatabase, const CameraScanResult& result )
{
	{
		std::lock_guard< std::mutex > lock( m_CameraScanResultsMutex );
		m_CameraScanResults.push_front( result );
		if ( m_CameraScanResults.size() > 100 )
		{
			m_CameraScanResults.pop_back();
		}
	}	

	std::stringstream wss;
	wss << "UPDATE WebServers SET Scanned=1 WHERE IP='" << result.address.GetHostAsString() << "';";
	ExecuteDatabaseQuery( pDatabase, wss.str() );

	if ( result.isCamera )
	{
		std::stringstream updateCameraQuery;
		updateCameraQuery << "UPDATE Cameras SET Geo=1 WHERE IP='" << result.address.ToString() << "';";
		ExecuteDatabaseQuery( pDatabase, updateCameraQuery.str() );

		sqlite3_stmt* pAddCameraStatement;
		const char* pAddCameraQuery = "INSERT OR REPLACE INTO Cameras VALUES(?1, ?2, ?3, ?4, ?5);";
		sqlite3_prepare_v2( pDatabase, pAddCameraQuery, -1, &pAddCameraStatement, nullptr );

		sqlite3_bind_text( pAddCameraStatement, 1, result.address.GetHostAsString().c_str(), -1, SQLITE_TRANSIENT );
		sqlite3_bind_int( pAddCameraStatement, 2, result.address.GetPort() );
		sqlite3_bind_int( pAddCameraStatement, 3, 0 ); // Type (unused at the moment).
		sqlite3_bind_text( pAddCameraStatement, 4, result.title.c_str(), -1, SQLITE_TRANSIENT );
		sqlite3_bind_int( pAddCameraStatement, 5, 0 ); // Geolocation pending.
		ExecuteDatabaseQuery( pDatabase, pAddCameraStatement );
		sqlite3_finalize( pAddCameraStatement );
	}
	else
	{
		printf("No camera detected at %s, title: %s\n", result.address.ToString().c_str(), result.title.c_str() );

		std::ofstream titleFile;
		titleFile.open("titles.txt", std::ios_base::app);
		titleFile << result.address.ToString() << ": " << result.title << "\n";
	}
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