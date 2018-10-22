#include <sstream>
#include <chrono>
#include "internet_scanner_basic.h"
#include "port_probe.h"
#include "watcher.h"

bool InternetScannerBasic::ScanInternal( Network::IPAddress address, const Network::PortVector& ports )
{
	for ( unsigned short port : ports )
	{
		address.SetPort( port );
		SetStatusText( "Probing " + address.ToString() );
		PortProbe probe;
		if ( probe.Probe( address ) == PortProbe::Result::Open )
		{
			g_pWatcher->OnWebServerFound( m_pDatabase, address );
		}
	}
	
	return true;
}
