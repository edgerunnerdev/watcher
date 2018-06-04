#include <iostream>
#include <fstream>
#include <sstream>
#include "imgui/imgui.h"
#include "sqlite/sqlite3.h"
#include "ip_generator.h"
#include "watcher.h"
#include "scanner.h"

Watcher::Watcher( unsigned int scannerCount ) :
m_Active( true ),
m_pDatabase( nullptr ),
m_ConfigInitialIP( { 1, 0, 0, 1 }, 0 )
{
	ReadConfig();
	m_pIPGenerator = std::make_unique< IPGenerator >( m_ConfigInitialIP );
	sqlite3_open( "0x00-watcher.db", &m_pDatabase );

	auto scannerThreadMain = []( Watcher* pWatcher, Scanner* pScanner )
	{
		while ( pWatcher->IsActive() )
		{
			Network::IPAddress address = pWatcher->GetIPGenerator()->GetNext();
			if ( pScanner->Scan( address ) )
			{
				pWatcher->AddEntry( address );
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

	ImGui::End();
}

void Watcher::AddEntry( Network::IPAddress address )
{
	std::lock_guard< std::mutex > lock( m_KnownServersMutex );
	m_KnownServers.push_back( address );

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