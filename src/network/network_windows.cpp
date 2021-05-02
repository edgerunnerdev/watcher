#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cassert>

#include "network.h"

namespace Network
{

Result ToResult( int result );

Result Initialise()
{
	WSADATA wsaData;
	return ToResult( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) );
}

Result Shutdown()
{
	return ToResult( WSACleanup() );
}

Result ConnectTCP( IPAddress address, unsigned int timeout, TCPSocket& tcpSocket )
{
	tcpSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( tcpSocket == INVALID_SOCKET )
	{
		return ToResult( WSAGetLastError() );
	}

	sockaddr_in addr;
	memset( &addr, 0, sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons( address.GetPort() );
	addr.sin_addr.s_addr = htonl( address.GetHost() );

	if ( timeout > 0u )
	{
		// Set the socket to non-blocking.
		unsigned long mode = 1u;
		ioctlsocket( tcpSocket, FIONBIO, &mode );

		connect( tcpSocket, (sockaddr*)&addr, sizeof( addr ) );

		fd_set fdset;
		FD_ZERO( &fdset );
		FD_SET( tcpSocket, &fdset );

		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		int selectResult = select( tcpSocket + 1, nullptr, &fdset, nullptr, &tv );
		if ( selectResult == 0 )
		{
			Close( tcpSocket );
			return Result::Timeout;
		}
		else if ( selectResult == 1 )
		{
			char soError;
			int len = static_cast< int >( sizeof( soError ) );

			if ( getsockopt( tcpSocket , SOL_SOCKET, SO_ERROR, &soError, &len ) == SOCKET_ERROR )
			{
				Close( tcpSocket );
				return ToResult( WSAGetLastError() );
			}

			if ( soError == 0 ) 
			{
				return Result::Success;
			}
			else
			{
				Close( tcpSocket );
				return ToResult( soError );
			}
		}
		else
		{
			int selectError = WSAGetLastError();
			Close( tcpSocket );
			return ToResult( selectError );
		}
	}
	else
	{
		if ( connect( tcpSocket, (sockaddr*)&addr, sizeof(addr)) == 0 )
		{
			return Result::Success;
		}
		else
		{
			Close( tcpSocket );
			return ToResult( WSAGetLastError() );
		}
	}
}

Result Close( TCPSocket socket )
{
	if ( closesocket( socket ) == 0 )
	{
		return Result::Success;
	}
	else
	{
		return ToResult( WSAGetLastError() );
	}
}

Result Resolve( const std::string& host, IPAddress& address )
{
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* pResult = nullptr;
	struct addrinfo* pPtr = nullptr;

	DWORD retVal = getaddrinfo(host.c_str(), nullptr, &hints, &pResult);
	if (retVal == 0)
	{
		for (struct addrinfo* pData = pResult; pData != nullptr; pData = pData->ai_next)
		{
			if (pData->ai_family == AF_INET)
			{
				struct sockaddr_in* pSockAddr = reinterpret_cast<struct sockaddr_in*>(pData->ai_addr);
				char buffer[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &pSockAddr->sin_addr, buffer, INET_ADDRSTRLEN) != nullptr)
				{
					address = IPAddress(buffer);
				}
				return Result::Success;
			}
		}
	}

	return Result::Unknown;
}

Result ToResult( int result )
{
	if ( result == NO_ERROR ) return Result::Success;
	else if ( result == WSASYSNOTREADY ) return Result::NotReady;
	else if ( result == WSAVERNOTSUPPORTED ) return Result::NotSupported;
	else if ( result == WSANOTINITIALISED ) return Result::NotInitialised;
	else if ( result == WSAENETDOWN ) return Result::NetDown;
	else if ( result == WSAEINPROGRESS ) return Result::InProgress;
	else if ( result == WSAENOTSOCK ) return Result::NotSocket;
	else if ( result == WSAECONNREFUSED ) return Result::ConnectionRefused;
	else if ( result == WSAEHOSTUNREACH ) return Result::HostUnreachable;
	else if ( result == WSAETIMEDOUT ) return Result::Timeout;
	else
	{
		printf("Unlisted error %d\n", result );
		assert( false );
		return Result::Unknown;
	}
}

std::string ToString( Result result )
{
	if ( result == Result::Success ) return "Success";
	else if ( result == Result::NotReady ) return "Network subsystem unavailable";
	else if ( result == Result::NotSupported ) return "Winsock.dll version out of range";
	else if ( result == Result::NotInitialised ) return "Successful WSAStartup not yet performed";
	else if ( result == Result::NetDown ) return "Network is down.";
	else if ( result == Result::InvalidFileDescriptor ) return "Invalid file descriptor";
	else if ( result == Result::NotSocket ) return "Not a socket";
	else if ( result == Result::NotConnected ) return "Not connected";  
	else if ( result == Result::PermissionDenied ) return "Permission denied";
	else if ( result == Result::AddressFamilyNotSupported ) return "Address family not supported";
	else if ( result == Result::Invalid ) return "Invalid";
	else if ( result == Result::PerProcessLimitReached ) return "Per process limit on the number of open sockets reached";
	else if ( result == Result::SystemLimitReached ) return "System limit on the number of open sockets reached";
	else if ( result == Result::InsufficientMemory ) return "Insufficient memory";
	else if ( result == Result::ConnectionRefused ) return "Connection refused";
	else if ( result == Result::HostUnreachable ) return "Host unreachable";
	else if ( result == Result::Timeout ) return "Timeout";
	else return "Unknown";	
}

}

#endif