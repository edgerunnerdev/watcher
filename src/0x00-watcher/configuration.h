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

	const Network::PortVector& GetWebScannerPorts() const;
	void SetWebScannerPorts( const Network::PortVector& ports );

private:
	void Save();
	void Load();
	void UseDefaults();

	Network::IPAddress m_StartAddress;
	int m_Rate;
	Network::PortVector m_Ports;
};
