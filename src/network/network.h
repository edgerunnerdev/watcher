#pragma once

#include <string>
#include <vector>

namespace Network
{

//-----------------------------------------------------------------------------
// IPAddress
// All return values are in host order, not network order.
//-----------------------------------------------------------------------------
class IPAddress
{
public:
	IPAddress( const std::vector< int >& host, unsigned short port );
	IPAddress( const std::string& address );
	void SetHost( const std::vector< int >& host );
	unsigned int GetHost() const;
	void SetPort( unsigned short port );
	unsigned short GetPort() const;
	std::string ToString() const;

private:
	unsigned int m_Host;
	unsigned short m_Port;
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