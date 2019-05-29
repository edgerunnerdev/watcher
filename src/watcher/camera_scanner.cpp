#include <fstream>
#include <string.h>
#include <curl/curl.h>
#include "ext/htmlstreamparser.h"
#include "ext/json.h"
#include "network/network.h"
#include "camera_scanner.h"
#include "watcher.h"

struct CameraScannerCallbackData
{
	HTMLSTREAMPARSER* pHsp;
	CameraScanner* pCameraScanner;
};

static size_t write_callback( void* buffer, size_t size, size_t nmemb, void* pData )
{
	CameraScannerCallbackData* pCallbackData = reinterpret_cast< CameraScannerCallbackData* >( pData );
	HTMLSTREAMPARSER* pHsp = pCallbackData->pHsp;
	size_t realsize = size * nmemb;
	std::string titleEndTag = "/title";
	size_t titleLen = 0;
	for (size_t p = 0; p < realsize; p++)
	{
		char c = ((char*)buffer)[p];
		if ( c <= -1 )
		{
			continue;
		}

		html_parser_char_parse( pHsp, ((char *)buffer)[p]);
		if ( html_parser_cmp_tag( pHsp, &titleEndTag[0], titleEndTag.size() ) )
		{
			titleLen = html_parser_inner_text_length( pHsp );
			char* pTitle = html_parser_replace_spaces( html_parser_trim( html_parser_inner_text( pHsp ), &titleLen ), &titleLen );
			
			if ( titleLen > 0 )
			{
				pTitle[ titleLen ] = '\0';
				pCallbackData->pCameraScanner->SetTitle( pTitle );
			}
		}
	}
	return realsize;
}

CameraScanner::CameraScanner()
{
	m_Title = "";

	LoadCameraDetectionRules();
}

CameraScanResult CameraScanner::Scan( Network::IPAddress address )
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

	CameraScannerCallbackData data = { hsp, this };
	std::string url = "http://" + address.ToString();
	curl_easy_setopt( pCurl, CURLOPT_URL, url.c_str() );
	curl_easy_setopt( pCurl, CURLOPT_WRITEFUNCTION, write_callback );
	curl_easy_setopt( pCurl, CURLOPT_WRITEDATA, &data );
	curl_easy_setopt( pCurl, CURLOPT_FOLLOWLOCATION, 1L );
	curl_easy_setopt( pCurl, CURLOPT_TIMEOUT, 10L );

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);

	CameraScanResult result;
	result.isCamera = EvaluateDetectionRules( url );
	result.address = address;
	result.title = m_Title;
	return std::move( result );
}

void CameraScanner::SetTitle( const std::string& title )
{
	m_Title = title;
}

void CameraScanner::LoadCameraDetectionRules()
{
	using json = nlohmann::json;
	std::ifstream file( "rules/cameradetection.json" );
	if ( file.is_open() )
	{
		json jsonRules;
		file >> jsonRules;
		file.close();

		for ( auto& jsonRule : jsonRules ) 
		{
			CameraDetectionRule cameraDetectionRule;
			for ( json::iterator it = jsonRule.begin(); it != jsonRule.end(); ++it ) 
			{
				const std::string& key = it.key();
				if ( key == "intitle" )
				{
					if ( it.value().is_array() )
					{
						for ( auto& text : it.value() )
						{
							cameraDetectionRule.AddFilter( CameraDetectionRule::FilterType::InTitle, text );
						}
					}
				}
				else if ( key == "pageexists" )
				{
					cameraDetectionRule.AddFilter( CameraDetectionRule::FilterType::PageExists, it.value() );
				}
			}
			m_CameraDetectionRules.push_back( cameraDetectionRule );
		}
	}
}

bool CameraScanner::EvaluateDetectionRules( const std::string& url ) const
{
	for ( auto& rule : m_CameraDetectionRules )
	{
		if ( rule.Match( url, m_Title ) )
		{
			return true;
		}
	}
	return false;
}