#include <sstream>
#include <chrono>
#include "ext/sqlite/sqlite3.h"
#include "internet_scanner.h"

InternetScanner::InternetScanner()
{
	sqlite3_open( "0x00-watcher.db", &m_pDatabase );
	sqlite3_busy_timeout( m_pDatabase, 1000 );

	SetStatusText( "Inactive" );
}

InternetScanner::~InternetScanner()
{
	sqlite3_close( m_pDatabase );
}

bool InternetScanner::Scan( Network::IPAddress address, const Network::PortVector& ports )
{
	return ScanInternal( address, ports );
}

std::string InternetScanner::GetStatusText() const
{
	std::string statusText;
	std::lock_guard< std::mutex > lock( m_StatusTextMutex );
	statusText = m_StatusText;
	return statusText;
}

void InternetScanner::SetStatusText( const std::string& statusText )
{
	std::lock_guard< std::mutex > lock( m_StatusTextMutex );
	m_StatusText = statusText;
}
