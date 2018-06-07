#include <string.h>
#include <curl/curl.h>
#include "ext/htmlstreamparser.h"
#include "network/network.h"
#include "camera_scanner.h"
#include "watcher.h"

CameraScanner* g_pCameraScanner = nullptr;

static size_t write_callback( void* buffer, size_t size, size_t nmemb, void* data )
{
	HTMLSTREAMPARSER* hsp = reinterpret_cast< HTMLSTREAMPARSER* >( data );
	size_t realsize = size * nmemb;
	std::string titleEndTag = "/title";
	size_t titleLen = 0;
	for (size_t p = 0; p < realsize; p++)
	{
		char c = ((char*)buffer)[p];

		html_parser_char_parse(hsp, ((char *)buffer)[p]);
		if ( html_parser_cmp_tag( hsp, &titleEndTag[0], titleEndTag.size() ) )
		{
			titleLen = html_parser_inner_text_length( hsp );
			char* pTitle = html_parser_replace_spaces( html_parser_trim( html_parser_inner_text( hsp ), &titleLen ), &titleLen );
			
			if ( titleLen > 0 )
			{
				pTitle[ titleLen ] = '\0';
				g_pCameraScanner->SetTitle( pTitle );
			}
		}
	}
	return realsize;
}

CameraScanner::CameraScanner()
{
	m_Title = "";
	g_pCameraScanner = this;
}

bool CameraScanner::Scan( Network::IPAddress address )
{
	CURL* pCurl = curl_easy_init();

	char tagBuffer[32];
	char attrBuffer[32];
	char valBuffer[128];
	char innerTextBuffer[1024];
	HTMLSTREAMPARSER* hsp = html_parser_init();

	html_parser_set_tag_to_lower( hsp, 1 );
	html_parser_set_attr_to_lower( hsp, 1 );
	html_parser_set_tag_buffer( hsp, tagBuffer, sizeof( tagBuffer ) );
	html_parser_set_attr_buffer( hsp, attrBuffer, sizeof( attrBuffer ) );
	html_parser_set_val_buffer( hsp, valBuffer, sizeof( valBuffer ) - 1 );
	html_parser_set_inner_text_buffer( hsp, innerTextBuffer, sizeof( innerTextBuffer ) - 1 ); 

	std::string url = "http://" + address.ToString();
	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, hsp);
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 20L);

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);

	const CameraDetectionRuleVector& rules = g_pWatcher->GetCameraDetectionRules();
	if ( m_Title.length() > 0 )
	{
		printf("%s: %s\n", url.c_str(), m_Title.c_str());
	}

	return false;
}

void CameraScanner::SetTitle( const std::string& title )
{
	m_Title = title;
}