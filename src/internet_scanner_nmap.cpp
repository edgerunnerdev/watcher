#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include "ext/sqlite/sqlite3.h"
#include "watcher.h"
#include "internet_scanner_nmap.h"

#include <stdio.h>
#include <stdlib.h>

bool InternetScannerNmap::IsSupported()
{
#ifdef __linux__
	return true;
#else
	return false;
#endif
}

bool InternetScannerNmap::ScanInternal( Network::IPAddress address, const Network::PortVector& ports )
{
#ifdef __linux__
	unsigned int host = address.GetHost();
	std::stringstream nmapAddress;	nmapAddress << (host >> 24) << "." << ((host >> 16 ) & 0xFF) << ".*.*";

	SetStatusText( "Probing " + nmapAddress.str() );

	std::stringstream portss;
	for ( int i = 0; i < (int)ports.size(); ++i )
	{
		portss << ports[ i ];
		if ( i < ( i - 1 ) )
		{
			ports << ',';
		}
	}

	std::stringstream ss;
	ss << "/usr/bin/nmap " <<
		"-sS -Pn -n --open " <<
		"-p" << portss.str() << " " <<
		"--randomize-hosts " <<
		"--max-retries 0 " <<
		"--min-parallelism 64 " <<
		"--host-timeout 5s " <<
		"-T5 " <<
		nmapAddress.str();

	FILE* pStream = popen( ss.str().c_str(), "r" );
	char pOutputBuffer[ 1024 ] = { 0 };
	if ( pStream == nullptr )
	{
		fprintf( stderr, "pOpen failed\n" );
		return false;
	}

	Network::IPAddress resultAddress;
	while ( nullptr != fgets( pOutputBuffer, sizeof( pOutputBuffer ) - 1, pStream ) )
	{
  		if ( Parse( pOutputBuffer, resultAddress ) )
  		{
  			printf("Found potential HTTP server at %s\n", resultAddress.ToString().c_str() );
			g_pWatcher->OnWebServerFound( m_pDatabase, resultAddress );
  		}
	}

   	int termStatus = pclose( pStream );
   	if ( termStatus == -1 )
   	{
  		fprintf( stderr, "pclose failed\n" );
  		return false;
   	}

	return true;
#else
	return false;
#endif
}

bool InternetScannerNmap::Parse( const char* pText, Network::IPAddress& result )
{
#ifdef __linux__
	static Network::IPAddress currentAddress;

	char ipAddress[ 1024 ] = { 0 };
	int port = 0;
	if ( sscanf( pText, "Nmap scan report for %s", ipAddress) == 1 )
	{
		currentAddress = Network::IPAddress( ipAddress );
	}
	else if ( sscanf( pText, "%d/tcp", &port ) == 1 )
	{
		currentAddress.SetPort( static_cast< unsigned short >( port ) );
		result = currentAddress;
		return true;
	}

	return false;
#else
	return false;
#endif
}
