// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.

#include <cassert>
#include <string>
#include <sstream>

#include "port_probe.h"

PortProbe::Result PortProbe::Probe( const Network::IPAddress& address )
{
	using namespace Network;

	assert( address.GetPort() != 0 );
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