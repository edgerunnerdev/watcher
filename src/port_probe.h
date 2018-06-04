#pragma once

#include <string>

#include "network/network.h"

class PortProbe
{
public:
	enum class Result
	{
		Open,
		Closed,
		Timeout
	};

	Result Probe( const Network::IPAddress& address );
};

std::string ToString( PortProbe::Result result );