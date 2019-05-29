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

#include "httpscanner.h"
#include "portprobe.h"

void HTTPScanner::Go( const Options& options )
{
	IPGenerator generator( options.GetIPAddress() );
	auto threadMain = [ &generator, &options ]( BasicScan* pBasicScan )
	{
		PortProbe probe;
		Network::IPAddress address;
		while ( generator.GetNext( address ) )
		{
			for ( const PortRange& portRange : options.GetPortRanges() )
			{
				for ( uint16_t port = portRange.begin; port <= portRange.end; ++port )
				{
					address.SetPort( port );
					if ( probe.Probe( address ) == PortProbe::Result::Open )
					{
						printf( "%s\n", address.ToString().c_str() );
					}
				}
			}
		}
	};

	for ( int i = 0; i < options.GetThreadCount(); ++i )
	{
		m_Threads.emplace_back( threadMain, this );
	}

	for ( auto& thread : m_Threads )
	{
		if ( thread.joinable() )
		{
			thread.join();
		}
	}
}
