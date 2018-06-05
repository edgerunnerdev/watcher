#include <iostream>
#include <fstream>
#include <sstream>
#include "imgui/imgui.h"
#include "sqlite/sqlite3.h"
#include "ip_generator.h"
#include "json.h"
#include "watcher.h"
#include "scanner.h"

Watcher* g_pWatcher = nullptr;

Watcher::Watcher( unsigned int scannerCount ) :
m_Active( true ),
m_pDatabase( nullptr ),
m_ConfigInitialIP( { 1, 0, 0, 1 }, 0 )
{
	g_pWatcher = this;

	ReadConfig();
	m_pIPGenerator = std::make_unique< IPGenerator >( m_ConfigInitialIP );
	sqlite3_open( "0x00-watcher.db", &m_pDatabase );

	LoadCameraDetectionRules();
	PopulateCameraDetectionQueue();

	auto scannerThreadMain = []( Watcher* pWatcher, Scanner* pScanner )
	{
		std::vector< unsigned short > ports = { 80, 81, 83, 8080 };
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

Watcher::~Watcher()
{
	m_Active = false;
	for ( auto& thread : m_ScannerThreads )
	{
		thread.join();
	}

	sqlite3_close( m_pDatabase );

	WriteConfig();
}

void Watcher::Update()
{
	ImGui::SetNextWindowPos( ImVec2( 0, 0 ) );
	ImGui::SetNextWindowSize( ImVec2( 400, 0 ) );
	ImGui::Begin("LeftBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar );

	std::stringstream wss;
	unsigned int currentIP = m_pIPGenerator->GetCurrent().GetHost();
	unsigned int maxIP = ~0u;
	double ratio = static_cast< double >( currentIP ) / static_cast< double >( maxIP );
	float percent = static_cast< float >( ratio * 100.0f );
	wss.precision( 3 );
	wss <<  "Webserver scanner (" << percent << "%)";

	if ( ImGui::CollapsingHeader( wss.str().c_str() ) )
	{
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

	if ( ImGui::CollapsingHeader( "Camera scanner" ) )
	{
		std::stringstream queueSizeSS;
		queueSizeSS << "Queue size: " << m_CameraDetectionQueue.size();
		ImGui::Text( queueSizeSS.str().c_str() );

		std::stringstream rulesSS;
		rulesSS << "Rules loaded: " << m_CameraDetectionRules.size();
		ImGui::Text( rulesSS.str().c_str() );
	}

	ImGui::End();
}

void Watcher::OnWebServerFound( Network::IPAddress address )
{
	// Store this web server into the database, mark it as not parsed.
	std::stringstream ss;
	ss << "INSERT OR REPLACE INTO WebServers VALUES('" << address.ToString() << "', 0);";

	char* pError = nullptr;
	int rc = sqlite3_exec( m_pDatabase, ss.str().c_str() , nullptr, 0, &pError );
	if( rc != SQLITE_OK )
	{
		fprintf( stderr, "SQL error: %s\n", pError );
		sqlite3_free( pError );
	}

	{
		std::lock_guard< std::mutex > lock( m_CameraDetectionQueueMutex );
		m_CameraDetectionQueue.push_back( address );
	}
}

void Watcher::WriteConfig()
{
	std::ofstream configFile("0x00-watcher.cfg");
	configFile << m_pIPGenerator->GetCurrent().ToString() << "\n";
	configFile.close();
}

void Watcher::ReadConfig()
{
	std::ifstream configFile("0x00-watcher.cfg");
	if ( configFile.is_open() )
	{
		std::string addr;
		if ( getline( configFile, addr ) )
	    {
	    	m_ConfigInitialIP = Network::IPAddress( addr );
	    }

		configFile.close();
	}
}

void Watcher::LoadCameraDetectionRules()
{
	using json = nlohmann::json;
	std::ifstream file( "rules/cameradetection.json" );
	if ( file.is_open() )
	{
		json jsonRules;
		file >> jsonRules;
		file.close();

		for ( auto& jsonRule : jsonRules ) 
		{
			CameraDetectionRule cameraDetectionRule;
			for ( json::iterator it = jsonRule.begin(); it != jsonRule.end(); ++it ) 
			{
				const std::string& key = it.key();
				if ( key == "intitle" )
				{
					if ( it.value().is_array() )
					{
						for ( auto& text : it.value() )
						{
							cameraDetectionRule.AddFilter( CameraDetectionRule::FilterType::InTitle, text );
						}
					}
				}
	  		}
			m_CameraDetectionRules.push_back( cameraDetectionRule );
		}
	}
}

// Loads from the database any IP addresses for web servers which have been
// found but not processed yet and adds them to the camera detection queue.
// Any servers which are identified at run time are added to the queue via the
// OnWebServerFound() callback instead.
void Watcher::PopulateCameraDetectionQueue()
{
	auto callback = []( void* notUsed, int argc, char** argv, char** azColName )
	{
		for ( int i = 0; i < argc; i++ )
		{
			Network::IPAddress ipAddress( argv[i] );
			g_pWatcher->OnWebServerAddedFromDatabase( ipAddress );
		}
		return 0;
	};

	std::string query = "SELECT IP FROM WebServers WHERE Parsed=0";
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
	m_CameraDetectionQueue.push_back( address );
}