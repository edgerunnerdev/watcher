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
#include "ipgenerator.h"

HTTPScanner::HTTPScanner() :
	m_ActiveThreads(0),
	m_Stop(false)
{

}

HTTPScanner::~HTTPScanner()
{
	m_Stop = true;

	for (auto& thread : m_Threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
}

int HTTPScanner::Go( Network::IPAddress block, int numThreads, const std::vector< uint16_t >& ports  )
{
	//SDL_assert( m_ActiveThreads == 0 );
	m_ActiveThreads = numThreads;
	m_Stop = false;

	m_pIPGenerator = std::make_unique<IPGenerator>( block );
	int remaining = m_pIPGenerator->GetRemaining();
	auto threadMain = [ numThreads, &ports ]( HTTPScanner* pHTTPScanner )
	{
		PortProbe probe;
		Network::IPAddress address;
		while ( pHTTPScanner->m_pIPGenerator->GetNext( address ) )
		{
			for ( uint16_t port : ports )
			{
				address.SetPort( port );
				if ( probe.Probe( address ) == PortProbe::Result::Open )
				{
					printf( "%s\n", address.ToString().c_str() );
				}

				if ( pHTTPScanner->m_Stop )
				{
					pHTTPScanner->m_ActiveThreads--;
					return;
				}
			}
		}

		pHTTPScanner->m_ActiveThreads--;
	};

	for ( int i = 0; i < numThreads; ++i )
	{
		m_Threads.emplace_back( threadMain, this );
	}

	return remaining;
}

bool HTTPScanner::IsScanning() const
{
	return m_ActiveThreads > 0;
}

void HTTPScanner::Stop()
{
	m_Stop = true;
}

int HTTPScanner::GetRemaining() const
{
	return m_pIPGenerator ? m_pIPGenerator->GetRemaining() : 0;
}
