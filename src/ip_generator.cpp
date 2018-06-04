#include "ip_generator.h"

IPGenerator::IPGenerator( Network::IPAddress startAddress )
{
	unsigned int host = startAddress.GetHost();
	m_Current = 
	{
		(int)( host >> 24 ),
		(int)( ( host >> 16 ) & 0xFF ),
		(int)( ( host >> 8 ) & 0xFF ),
		(int)( host & 0xFF ) 
	};
}

Network::IPAddress IPGenerator::GetNext()
{
	std::lock_guard< std::mutex > lock( m_Mutex );

	m_Current[ 3 ]++;
	if ( m_Current[ 3 ] > 255 )
	{
		m_Current[ 3 ] = 0;
		m_Current[ 2 ]++;
	}

	if ( m_Current[ 2 ] > 255 )
	{
		m_Current[ 2 ] = 0;
		m_Current[ 1 ]++;
	}

	if ( m_Current[ 1 ] > 255 )
	{
		m_Current[ 1 ] = 0;
		m_Current[ 0 ]++;
	}

	if ( m_Current[ 0 ] > 255 )
	{
		m_Current = { 1, 0, 0, 0 };
	}

	// Range skipping rules.
	// 10.0.0.0 - 10.255.255.255, private network.
	if ( m_Current[ 0 ] == 10 )
	{
		m_Current[ 0 ] = 11;
	}
	// 100.64.0.0 - 100.127.255.255, private network.
	else if ( m_Current[ 0 ] == 100 && m_Current[ 1 ] == 64 )
	{
		m_Current[ 1 ] = 128;
	}
	// 127.0.0.0 - 127.255.255.255, loopback.
	else if ( m_Current[ 0 ] == 127 )
	{
		m_Current[ 0 ] = 128;
	}

	// TODO:
	//169.254.0.0/16	169.254.0.0–169.254.255.255	65536	Subnet	Used for link-local addresses[4] between two hosts on a single link when no IP address is otherwise specified, such as would have normally been retrieved from a DHCP server.
	//172.16.0.0/12	172.16.0.0–172.31.255.255	1048576	Private network	Used for local communications within a private network.[2]
	//192.0.0.0/24	192.0.0.0–192.0.0.255	256	Private network	IETF Protocol Assignments.[1]
	//192.0.2.0/24	192.0.2.0–192.0.2.255	256	Documentation	Assigned as TEST-NET-1, documentation and examples.[5]
	//192.88.99.0/24	192.88.99.0–192.88.99.255	256	Internet	Reserved.[6] Formerly used for IPv6 to IPv4 relay[7] (included IPv6 address block 2002::/16).
	//192.168.0.0/16	192.168.0.0–192.168.255.255	65536	Private network	Used for local communications within a private network.[2]
	//198.18.0.0/15	198.18.0.0–198.19.255.255	131072	Private network	Used for benchmark testing of inter-network communications between two separate subnets.[8]
	//198.51.100.0/24	198.51.100.0–198.51.100.255	256	Documentation	Assigned as TEST-NET-2, documentation and examples.[5]
	//203.0.113.0/24	203.0.113.0–203.0.113.255	256	Documentation	Assigned as TEST-NET-3, documentation and examples.[5]
	//224.0.0.0/4	224.0.0.0–239.255.255.255	268435456	Internet	In use for IP multicast.[9] (Former Class D network).
	//240.0.0.0/4	240.0.0.0–255.255.255.254	268435456	Internet	Reserved for future use.[10] (Former Class E network).
	//255.255.255.255/32	255.255.255.255	1	Subnet	Reserved for the "limited broadcast" destination address.[1][11]

	return Network::IPAddress( m_Current, 0 );
}

Network::IPAddress IPGenerator::GetCurrent()
{
	std::lock_guard< std::mutex > lock( m_Mutex );
	return Network::IPAddress( m_Current, 0 );
}	