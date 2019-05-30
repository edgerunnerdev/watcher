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

#pragma once

#include <string>
#include <vector>

// windows.h redefines SetPort as either SetPortA or SetPortW, which will cause
// issues with IPAddress::SetPort().
#ifdef _WIN32
#undef SetPort
#endif

namespace Network
{

using PortVector = std::vector< unsigned short >;

//-----------------------------------------------------------------------------
// IPAddress
// All return values are in host order, not network order.
//-----------------------------------------------------------------------------
class IPAddress
{
public:
	IPAddress();
	IPAddress( const std::vector< int >& host, uint16_t port );
	IPAddress( uint32_t host, uint16_t port );
	IPAddress( const std::string& address );
	void SetHost( const std::vector< int >& host );
	uint32_t GetHost() const;
	void SetPort( uint16_t port );
	uint16_t GetPort() const;
	void SetBlock( uint8_t block );
	uint8_t GetBlock() const;
	std::string ToString() const;
	std::string GetHostAsString() const;

	static bool IsValid( const std::string& address );

private:
	uint32_t m_Host;
	uint16_t m_Port;
	uint8_t m_Block;
};


using TCPSocket = int;


//-----------------------------------------------------------------------------
// Result
// Potential results for network operations. Not all platforms necessarily
// returns every value.
//-----------------------------------------------------------------------------
enum class Result
{
	Success,
	NotReady,
	NotSupported,
	NotInitialised,
	NetDown,
	InProgress,
	InvalidFileDescriptor,
	NotSocket,
	NotConnected,
	Timeout,
	PermissionDenied,
	AddressFamilyNotSupported,
	PerProcessLimitReached,
	SystemLimitReached,
	InsufficientMemory,
	Invalid,
	ConnectionRefused,
	HostUnreachable,
	Unknown
};


//-----------------------------------------------------------------------------
// Network API
//-----------------------------------------------------------------------------

Result Initialise();
Result Shutdown();
Result ConnectTCP( IPAddress address, unsigned int timeout, TCPSocket& tcpSocket );
Result Close( TCPSocket socket );

std::string ToString( Result result );

}