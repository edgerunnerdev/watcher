#include <cassert>
#include <sstream>
#include "network.h"

namespace Network
{

//-----------------------------------------------------------------------------
// IPAddress
//-----------------------------------------------------------------------------

IPAddress::IPAddress() :
m_Host( 0 ),
m_Port( 0 )
{

}

IPAddress::IPAddress( const std::vector< int >& host, unsigned short port )
{
	SetHost( host );
	SetPort( port );
}

IPAddress::IPAddress( unsigned int host, unsigned short port )
{
	m_Host = host;
	SetPort( port );
}

IPAddress::IPAddress( const std::string& address )
{
#ifdef _WIN32
#define sscanf sscanf_s
#endif

	std::vector< int > v = { 0, 0, 0, 0 };
	int port = 0;
	if ( sscanf( address.c_str(), "%d.%d.%d.%d:%d", &v[0], &v[1], &v[2], &v[3], &port ) == 5 )
	{
		SetHost( v );
		SetPort( port );
	}
	else if ( sscanf( address.c_str(), "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3] ) == 4 )
	{
		SetHost( v );
		SetPort( 0 );
	}
	else
	{
		// Invalid format
		assert( false );
	}
#ifdef _WIN32
#undef sscanf
#endif
}

void IPAddress::SetHost( const std::vector< int >& host )
{
	// Convert the vector into a single unsigned integer in host order.
	assert( host.size() == 4 );
	m_Host = ( host[ 0 ] << 24 ) | ( host[ 1 ] << 16 ) | ( host[ 2 ] << 8 ) | ( host[ 3 ] );
}

unsigned int IPAddress::GetHost() const 
{ 
	return m_Host; 
}

void IPAddress::SetPort( unsigned short port )
{
	m_Port = port;
}

unsigned short IPAddress::GetPort() const 
{ 
	return m_Port; 
}

std::string IPAddress::ToString() const
{
	std::stringstream ss;
	ss << GetHostAsString();

	if ( m_Port != 0u )
	{
		ss << ":" << m_Port;
	}

	return ss.str();
}

std::string IPAddress::GetHostAsString() const
{
	std::stringstream ss;
	ss << 
		( m_Host >> 24 ) << "." <<
		( ( m_Host >> 16 ) & 0xFF ) << "." <<
		( ( m_Host >> 8 ) & 0xFF ) << "." <<
		( m_Host & 0xFF );
	return ss.str();
}

}