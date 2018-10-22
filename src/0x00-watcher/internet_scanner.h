#pragma once

#include <string>
#include <mutex>

#include "network/network.h"

struct sqlite3;

class InternetScanner
{
public:
	InternetScanner();
	virtual ~InternetScanner();
	bool Scan( Network::IPAddress address, const Network::PortVector& ports );
	std::string GetStatusText() const;

protected:
	virtual bool ScanInternal( Network::IPAddress address, const Network::PortVector& ports ) = 0;

	void SetStatusText( const std::string& text );
	std::string m_StatusText;
	mutable std::mutex m_StatusTextMutex;
	sqlite3* m_pDatabase;
};
