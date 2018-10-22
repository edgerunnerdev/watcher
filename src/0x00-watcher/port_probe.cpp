#include <string>
#include <sstream>

#include <SDL.h>

#include "port_probe.h"

PortProbe::Result PortProbe::Probe( const Network::IPAddress& address )
{
	using namespace Network;

	SDL_assert( address.GetPort() != 0 );
	TCPSocket socket;
	Network::Result result = ConnectTCP( address, 2500, socket );

	if ( result == Network::Result::Success )
	{
		Close( socket );
		return Result::Open;
	}
	else if ( result == Network::Result::Timeout || result == Network::Result::HostUnreachable )
	{
		return PortProbe::Result::Timeout;
	}
	else if ( result == Network::Result::ConnectionRefused )
	{
		return PortProbe::Result::Closed;
	}
	else
	{
		printf("ConnectTCP error: %s\n", ToString(result).c_str());
		return PortProbe::Result::Timeout;
	}
}

std::string ToString( PortProbe::Result result )
{
	if ( result == PortProbe::Result::Open )
	{
		return "Open";
	}
	else if ( result == PortProbe::Result::Closed )
	{
		return "Closed";
	}
	else if ( result == PortProbe::Result::Timeout )
	{
		return "Timeout";
	}
	else
	{
		return "Unknown";
	}
}