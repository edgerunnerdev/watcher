///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <string>
#include <SDL.h>
#include "ext/json.h"
#include "configuration.h"

namespace Watcher
{

Configuration::Configuration()
{
	UseDefaults();
	Load();
}

Configuration::~Configuration()
{
	Save();
}

void Configuration::Save()
{
	using json = nlohmann::json;
	json config;
	config[ "start_address" ] = m_StartAddress.GetHostAsString();
	config[ "rate" ] = m_Rate;
	config[ "ports" ] = m_Ports;

	std::ofstream file( "config.json" );
	file << config;
	file.close();
}

void Configuration::Load()
{
	using json = nlohmann::json;
	std::ifstream file( "config.json" );
	if ( file.is_open() )
	{
		json config;
		file >> config;
		file.close();

		std::string startAddress = config[ "start_address" ];
		m_StartAddress = Network::IPAddress( startAddress );
		m_Rate = config[ "rate" ];

		m_Ports.clear();
		for ( json::iterator it = config.begin(); it != config.end(); ++it ) 
		{
			const std::string& key = it.key();
			if ( key == "ports" )
			{
				if ( it.value().is_array() )
				{
					for ( auto& port : it.value() )
					{
						SDL_assert( port >= 0 );
						SDL_assert( port <= 65535 );
						m_Ports.push_back( static_cast< unsigned short >( port ) );
					}
				}
			}
		}
	}
}

void Configuration::UseDefaults()
{
	m_StartAddress = Network::IPAddress( "1.0.0.0" );
	m_Rate = 100;
	m_Ports = { 80, 81, 8080 };
}

Network::IPAddress Configuration::GetWebScannerStartAddress() const
{
	return m_StartAddress;
}

void Configuration::SetWebScannerStartAddress( Network::IPAddress address )
{
	m_StartAddress = address;
}

int Configuration::GetWebScannerRate() const 
{
	return m_Rate;
}

void Configuration::SetWebScannerRate( int value )
{
	m_Rate = value;
}

const Network::PortVector& Configuration::GetWebScannerPorts() const
{
	return m_Ports;
}

void Configuration::SetWebScannerPorts( const Network::PortVector& ports )
{
	m_Ports = ports;
}

} // namespace Watcher
