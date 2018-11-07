#include <sstream>
#include <chrono>
#include "internet_scanner.h"

InternetScanner::InternetScanner()
{
	SetStatusText( "Inactive" );
}

InternetScanner::~InternetScanner()
{

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
