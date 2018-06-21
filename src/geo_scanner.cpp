#include <chrono>
#include <cstring>
#include <sstream>
#include <thread>
#include <curl/curl.h>
#include "sqlite/sqlite3.h"
#include "SDL.h"
#include "geo_info.h"
#include "geo_scanner.h"
#include "watcher.h"

// This needs to be a static function as libcurl is a C library and will segfault if passed
// a local lambda.
static size_t WriteMemoryCallback( void* pContents, size_t size, size_t nmemb, void* pUserData )
{
	size_t realSize = size * nmemb;
	std::string& data = *reinterpret_cast< std::string* >( pUserData );
	size_t curDataSize = data.size();
	data.resize( curDataSize + realSize );
	memcpy( &data[ curDataSize ], pContents, realSize );
	return realSize;
}

GeoScanner::GeoScanner()
{
	sqlite3_open( "0x00-watcher.db", &m_pDatabase );
	PopulateQueueFromDatabase();
	PopulateResultsFromDatabase();
	m_pCurlHandle = curl_easy_init();
	m_Data.reserve( 1024 );
}

GeoScanner::~GeoScanner()
{
	sqlite3_close( m_pDatabase );	
	curl_easy_cleanup( m_pCurlHandle );
}

void GeoScanner::Update()
{
	Network::IPAddress address;
	{
		std::lock_guard< std::mutex > lock( m_QueueMutex );
		if ( m_Queue.size() == 0 )
		{
			return;
		}
		else
		{
			address = m_Queue.back();
			m_Queue.pop_back();
		}
	}

	m_Data.clear();

	std::stringstream url;
	url << "https://ipinfo.io/" << address.ToString() << "/json"; 

	char pErrorBuffer[ CURL_ERROR_SIZE ];
	curl_easy_setopt( m_pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer );
	curl_easy_setopt( m_pCurlHandle, CURLOPT_URL, url.str().c_str() );
	curl_easy_setopt( m_pCurlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
	curl_easy_setopt( m_pCurlHandle, CURLOPT_WRITEDATA, &m_Data );
	curl_easy_setopt( m_pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
	curl_easy_setopt( m_pCurlHandle, CURLOPT_TIMEOUT, 10L );

	if ( curl_easy_perform( m_pCurlHandle ) != CURLE_OK )
	{
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"GeoScanner - error in curl",
			pErrorBuffer,
			nullptr
		);
	}
	else
	{
		GeoInfo geoInfo( address );
		if ( geoInfo.LoadFromJSON( m_Data ) )
		{
			geoInfo.SaveToDatabase( m_pDatabase );
			g_pWatcher->OnGeoInfoAdded( geoInfo );
		}
		else
		{
			SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_ERROR,
				"GeoScanner", "Error processing JSON response.",
				nullptr
			);
		}
	}
}

void GeoScanner::QueueScan( Network::IPAddress address )
{
	std::lock_guard< std::mutex > lock( m_QueueMutex );
	m_Queue.push_back( address );
}

void GeoScanner::PopulateQueueFromDatabase()
{
	auto callback = []( void* pOwner, int argc, char** argv, char** azColName )
	{
		GeoScanner* pGeoScanner = reinterpret_cast< GeoScanner* >( pOwner);
		for ( int i = 0; i < argc; i++ )
		{
			Network::IPAddress address( argv[i] );
			pGeoScanner->QueueScan( address );
		}
		return 0;
	};

	std::string query = "SELECT IP FROM Cameras WHERE Geo=0";
	char* pError = nullptr;
	int rc = sqlite3_exec( m_pDatabase, query.c_str(), callback, this, &pError );
	if( rc != SQLITE_OK )
	{
		fprintf( stderr, "SQL error: %s\n", pError );
		sqlite3_free( pError );
	}
}

void GeoScanner::PopulateResultsFromDatabase()
{
	auto callback = []( void* pOwner, int argc, char** argv, char** azColName )
	{
		Network::IPAddress address( argv[ 0 ] );
		std::string city = argv[ 1 ];
		std::string region = argv[ 2 ];
		std::string country = argv[ 3 ];
		std::string organisation = argv[ 4 ];
		float latitude = static_cast< float >( atof( argv[ 5 ] ) );
		float longitude = static_cast< float >( atof( argv[ 6 ] ) );
		GeoInfo geoInfo( address );
		geoInfo.LoadFromDatabase( city, region, country, organisation, latitude, longitude );
		g_pWatcher->OnGeoInfoAdded( geoInfo );
		return 0;
	};

	std::string query = "SELECT * FROM Geolocation";
	char* pError = nullptr;
	int rc = sqlite3_exec( m_pDatabase, query.c_str(), callback, this, &pError );
	if( rc != SQLITE_OK )
	{
		fprintf( stderr, "SQL error: %s\n", pError );
		sqlite3_free( pError );
	}	
}

size_t GeoScanner::GetQueueSize() const
{
	std::lock_guard< std::mutex > lock( m_QueueMutex );
	return m_Queue.size();
}

