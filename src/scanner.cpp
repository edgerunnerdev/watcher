#include <sstream>
#include <chrono>
#include "port_probe.h"
#include "scanner.h"

Scanner::Scanner()
{
	SetStatusText( "Inactive" );
}

bool Scanner::Scan( Network::IPAddress address )
{
	address.SetPort( 80 );
	SetStatusText( "Probing " + address.ToString() );
	PortProbe probe;
	if ( probe.Probe( address ) == PortProbe::Result::Open )
	{
		return true;
	}
	
	return false;
}

std::string Scanner::GetStatusText() const
{
	std::string statusText;
	std::lock_guard< std::mutex > lock( m_StatusTextMutex );
	statusText = m_StatusText;
	return statusText;
}

void Scanner::SetStatusText( const std::string& statusText )
{
	std::lock_guard< std::mutex > lock( m_StatusTextMutex );
	m_StatusText = statusText;
}
