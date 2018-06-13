#pragma once

#include <mutex>
#include <vector>

#include "network/network.h"

class IPGenerator
{
public:
	IPGenerator( Network::IPAddress startAddress );
	Network::IPAddress GetNext( int block = 32 );
	Network::IPAddress GetCurrent();
private:
	std::vector< int > m_Current;
	std::mutex m_Mutex;
};