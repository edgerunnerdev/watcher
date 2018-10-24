#include "geolocation.h"

#include <iostream>

DECLARE_PLUGIN( Geolocation, 0, 1, 0 )

Geolocation::Geolocation()
{

}

Geolocation::~Geolocation()
{
	if ( m_QueryThread.joinable() )
	{
		m_QueryThread.join();
	}
}

bool Geolocation::Initialise( PluginMessageCallback pMessageCallback )
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

void Geolocation::OnMessageReceived( const nlohmann::json& message )
{
	auto it = message.find( "geolocation_request" );
	if ( it != message.cend() )
	{
		{
			std::lock_guard< std::mutex > lock( m_QueueMutex );
			std::string value = *it;
			m_Queue.emplace_back( value );
		}

		ConsumeQueue();
	}
}

// If required, create a thread which will keep consuming the queue until it is
// empty. The thread is responsible for resolving the IP addresses into actual
// locations and for sending back the "geolocation_result" message.
void Geolocation::ConsumeQueue()
{
	if ( m_Queue.empty() || m_QueryThreadActive )
	{
		return;
	}

	auto threadMain = []( Geolocation* pGeolocation )
	{
		pGeolocation->m_QueryThreadActive = true;
		while ( 1 )
		{
			Network::IPAddress address;
			{
				std::lock_guard< std::mutex > lock( pGeolocation->m_QueueMutex );
				if ( pGeolocation->m_Queue.empty() )
				{
					break;
				}
				else
				{
					address = pGeolocation->m_Queue.back();
					pGeolocation->m_Queue.pop_back();
				}
			}

			int a = 0;
		}
		pGeolocation->m_QueryThreadActive = false;
	};

	// Ensure any previous thread is fully terminated before starting a new one.
	if ( m_QueryThread.joinable() )
	{
		m_QueryThread.join();
	}

	m_QueryThread = std::thread( threadMain, this );
}