#include <algorithm>
#include <cctype>
#include <curl/curl.h>
#include "camera_detection_rule.h"

static size_t write_callback( void* buffer, size_t size, size_t nmemb, void* pData )
{
	size_t realSize = size * nmemb;
	size_t* bytesReceived = reinterpret_cast< size_t* >( pData );
	*bytesReceived += realSize;
	return realSize;
}

bool CameraDetectionRule::Match( const std::string& url, const std::string& title ) const
{
	if ( title.size() == 0 )
	{
		return false;
	}

	const int inTitleRulesPassed = ProcessInTitleRules( title );
	const int inTextRulesPassed = ProcessInTextRules();
	const int pageExistsRulesPassed = ProcessPageExistsRules( url );

	return ( inTitleRulesPassed == m_InTitle.size() && inTextRulesPassed == m_InText.size() && pageExistsRulesPassed == m_PageExists.size() );
}

void CameraDetectionRule::AddFilter( FilterType type, const std::string& filter )
{
	std::string loweredFilter;
	loweredFilter.resize( filter.length() );
	std::transform( filter.begin(), filter.end(), loweredFilter.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });

	if ( type == FilterType::InTitle )
	{
		m_InTitle.push_back( loweredFilter );
	}
	else if ( type == FilterType::InText )
	{
		m_InText.push_back( loweredFilter );
	}
	else if ( type == FilterType::PageExists )
	{
		m_PageExists.push_back( filter );
	}
}

int CameraDetectionRule::ProcessInTitleRules( const std::string& title ) const
{
	std::string loweredTitle;
	loweredTitle.resize( title.length() );
	std::transform( title.begin(), title.end(), loweredTitle.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });

	int rulesPassed = 0;
	for ( const std::string& inTitleRule : m_InTitle )
	{
		if ( loweredTitle.find( inTitleRule ) != std::string::npos )
		{
			rulesPassed++;
		}
	}
	return rulesPassed;
}

int CameraDetectionRule::ProcessInTextRules() const
{
	// Not implemented.
	return 0;
}

int CameraDetectionRule::ProcessPageExistsRules( const std::string& url ) const
{
	if ( m_PageExists.empty() )
	{
		return 0;
	}

	int rulesPassed = 0;
	CURL* pCurl = curl_easy_init();
	for ( const std::string& page : m_PageExists )
	{
		size_t bytesReceived = 0u;
		std::string pageUrl = url + "/" + page;
		curl_easy_setopt( pCurl, CURLOPT_URL, pageUrl.c_str() );
		curl_easy_setopt( pCurl, CURLOPT_WRITEFUNCTION, write_callback );
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &bytesReceived );
		curl_easy_setopt( pCurl, CURLOPT_FOLLOWLOCATION, 1L );
		curl_easy_setopt( pCurl, CURLOPT_TIMEOUT, 10L );

		CURLcode ret = curl_easy_perform( pCurl );
		// Motion JPGs will trigger a timeout, but we'll have received data.
		if ( ret == CURLE_OK || ( ret == CURLE_OPERATION_TIMEDOUT && bytesReceived > 0u ) )
		{
			rulesPassed++;
		}
	}

	curl_easy_cleanup(pCurl);

	return rulesPassed;
}