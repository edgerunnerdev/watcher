#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <SDL.h>

#include "network.h"

namespace Network
{

Result ToResult( int result );

Result Initialise()
{
	return Result::Success;
}

Result Shutdown()
{
	return Result::Success;
}

Result ConnectTCP( IPAddress address, unsigned int timeout, TCPSocket& tcpSocket )
{
	tcpSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( tcpSocket == -1 )
	{
		return ToResult( errno );
	}

	sockaddr_in addr;
	memset( &addr, 0, sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons( address.GetPort() );
	addr.sin_addr.s_addr = htonl( address.GetHost() );

	if ( timeout > 0u )
	{
		fcntl( tcpSocket, F_SETFL, O_NONBLOCK );

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
			int soError;
        	socklen_t len = sizeof( soError );

        	if ( getsockopt( tcpSocket , SOL_SOCKET, SO_ERROR, &soError, &len ) == -1 )
        	{
        		Close( tcpSocket );
        		return ToResult( errno );
        	}

        	if (soError == 0) 
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
			int selectError = errno;
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
			return ToResult( errno );
		}
	}
}

Result Close( TCPSocket socket )
{
	if ( close( socket ) == 0 )
	{
		return Result::Success;
	}
	else
	{
		printf( "close error (%d): %s\n", errno, strerror( errno ) );
		return ToResult( errno );
	}
}

Result ToResult( int result )
{
	if ( result == 0 ) return Result::Success;
	else if ( result == EBADF ) return Result::InvalidFileDescriptor;
	else if ( result == ENOTSOCK ) return Result::NotSocket;
	else if ( result == ENOTCONN ) return Result::NotConnected;  
	else if ( result == EACCES ) return Result::PermissionDenied;
	else if ( result == EAFNOSUPPORT ) return Result::AddressFamilyNotSupported;
	else if ( result == EINVAL ) return Result::Invalid;
	else if ( result == EMFILE ) return Result::PerProcessLimitReached;
	else if ( result == ENFILE ) return Result::SystemLimitReached;
	else if ( result == ENOBUFS || result == ENOMEM ) return Result::InsufficientMemory;
	else if ( result == ECONNREFUSED ) return Result::ConnectionRefused;
	else if ( result == ENETUNREACH || result == EHOSTDOWN || result == EHOSTUNREACH ) return Result::HostUnreachable;
	else if ( result == ETIMEDOUT ) return Result::Timeout;
	else
	{
		printf("Unlisted error (%d): %s\n", result, strerror( result ) );
		SDL_assert( false );
		return Result::Unknown;
	}
}

std::string ToString( Result result )
{
	if ( result == Result::Success ) return "Success";
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