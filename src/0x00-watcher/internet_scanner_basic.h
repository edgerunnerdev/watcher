#pragma once

#include <string>
#include <mutex>

#include "network/network.h"
#include "internet_scanner.h"

class InternetScannerBasic : public InternetScanner
{
protected:
	virtual bool ScanInternal( Network::IPAddress address, const Network::PortVector& ) override;
};
