#pragma once

#include <mutex>
#include <string>
#include "network/network.h"
#include "internet_scanner.h"

class InternetScannerZmap : public InternetScanner
{
public:	
	static bool IsSupported();

private:
	virtual bool ScanInternal( Network::IPAddress address, const Network::PortVector& ports ) override;
	bool Parse( const char* pText, Network::IPAddress& result );
};
