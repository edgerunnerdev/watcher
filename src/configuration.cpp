#include <fstream>
#include <string>
#include "ext/json.h"
#include "configuration.h"

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
						m_Ports.push_back( port );
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
	m_Ports = { 80, 81, 82, 83, 84, 8080 };
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

std::vector< int > Configuration::GetWebScannerPorts() const
{
	return m_Ports;
}

void Configuration::SetWebScannerPorts( const std::vector< int >& ports )
{
	m_Ports = ports;
}
