#pragma once

#include <string>
#include <mutex>

#include "network/network.h"

class Scanner
{
public:
	Scanner();
	bool Scan( Network::IPAddress address );
	std::string GetStatusText() const;

private:
	void SetStatusText( const std::string& text );
	std::string m_StatusText;
	mutable std::mutex m_StatusTextMutex;
};