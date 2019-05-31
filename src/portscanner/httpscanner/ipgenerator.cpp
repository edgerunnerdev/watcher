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

#include "ipgenerator.h"

IPGenerator::IPGenerator( const Network::IPAddress& ipAddress )
{
	std::lock_guard< std::mutex > lock( m_Mutex );

	srand( static_cast< unsigned int >( time( nullptr ) ) );

	uint8_t block = ipAddress.GetBlock();
	uint32_t host = ipAddress.GetHost();

	// A CIDR block of less than 8 is meaningless - just use a single address.
	if ( block < 8 )
	{
		m_PossibleIPs.push_back( host );
	}
	else
	{
		// Create a mask representing the starting address for our desired block.
		uint32_t mask = ~( 0xFFFFFFFF >> block );
		host &= mask;

		// Generate the possible IP addresses we can provide.
		// This is required because we randomise the order in which we scan addresses,
		// so we need to keep track of what hasn't been scanned yet.
		size_t count = 1ULL << ( 32 - block );
		m_PossibleIPs.reserve( count );
		for ( size_t i = 0u; i < count; ++i )
		{
			m_PossibleIPs.push_back( host + static_cast< uint32_t >( i ) );
		}
	}

	m_RemainingIPs = m_PossibleIPs.size();
}

bool IPGenerator::GetNext( Network::IPAddress& ipAddress )
{
	std::lock_guard< std::mutex > lock( m_Mutex );

	if ( m_PossibleIPs.empty() )
	{
		return false;
	}
	else
	{
		const size_t numAddresses = m_PossibleIPs.size();
		const size_t idx = rand() % numAddresses;
		ipAddress = Network::IPAddress( m_PossibleIPs[ idx ], 0 );
		std::swap( m_PossibleIPs[ idx ], m_PossibleIPs[ numAddresses - 1 ] );
		m_PossibleIPs.pop_back();
		m_RemainingIPs--;
		return true;
	}
}

int IPGenerator::GetRemaining() const
{
	return m_RemainingIPs;
}
