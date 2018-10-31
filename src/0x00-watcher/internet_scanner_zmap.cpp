#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include "ext/sqlite/sqlite3.h"
#include "watcher.h"
#include "internet_scanner_zmap.h"

#include <stdio.h>
#include <stdlib.h>

bool InternetScannerZmap::IsSupported()
{
#ifdef __linux__
	return true;
#else
	return false;
#endif
}

bool InternetScannerZmap::ScanInternal( Network::IPAddress address, const Network::PortVector& ports )
{
#ifdef __linux__
	for ( int port : ports )
	{
		std::stringstream statusStream;
		statusStream << "Probing " << address.GetHostAsString() << "/16, port " << port;
		SetStatusText( statusStream.str() );

		std::stringstream ss;
		ss << "/usr/sbin/zmap " <<
			"--target-port=" << port << " " <<
			"-B 1M " <<
			address.GetHostAsString() <<
			"/16";

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
	  			resultAddress.SetPort( static_cast< unsigned short >( port ) );
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
	}

	return true;
#else
	return false;
#endif
}

bool InternetScannerZmap::Parse( const char* pText, Network::IPAddress& result )
{
#ifdef __linux__
	std::vector< int > addr = { 0, 0, 0, 0 };
	if ( sscanf( pText, "%d.%d.%d.%d", &addr[ 0 ], &addr[ 1 ], &addr[ 2 ], &addr[ 3 ] ) == 4 )
	{
		result = Network::IPAddress( addr, 0 );
		return true;
	}

	return false;
#else
	return false;
#endif
}
