#pragma once

#include <vector>
#include "network/network.h"

class Configuration
{
public:
	Configuration();
	~Configuration();

	Network::IPAddress GetWebScannerStartAddress() const;
	void SetWebScannerStartAddress( Network::IPAddress address );

	int GetWebScannerRate() const;
	void SetWebScannerRate( int value );

	std::vector< int > GetWebScannerPorts() const;
	void SetWebScannerPorts( const std::vector< int >& ports );

private:
	void Save();
	void Load();
	void UseDefaults();

	Network::IPAddress m_StartAddress;
	int m_Rate;
	std::vector< int > m_Ports;
};
