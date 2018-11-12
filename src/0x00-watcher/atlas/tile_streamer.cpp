#include <algorithm>
#include <chrono>
#include <sstream>

#include <curl/curl.h>

#include "atlas/atlas.h"
#include "atlas/tile_streamer.h"
#include "atlas/tile.h"
#include "filesystem.h"
#include "texture_loader.h"
#include "log.h"

namespace Atlas
{

TileStreamer::TileStreamer()
{
	m_RunThread = true;
	m_Thread = std::thread( &TileStreamer::TileStreamerThreadMain, this );
	CreateDirectories();
}

TileStreamer::~TileStreamer()
{
	if ( m_Thread.joinable() )
	{
		m_RunThread = false;
		m_Thread.join();
	}
}

TileSharedPtr TileStreamer::Get( int x, int y, int zoomLevel )
{
	auto containsFn = [ x, y, zoomLevel ]( TileSharedPtr pTile ) -> bool 
	{
		return pTile->X() == x && pTile->Y() == y && pTile->ZoomLevel() == zoomLevel;
	};

	std::lock_guard< std::mutex > lock( m_AccessMutex );
	auto loadedTileIt = std::find_if( m_LoadedTiles.begin(), m_LoadedTiles.end(), containsFn );
	if ( loadedTileIt != m_LoadedTiles.end() )
	{
		return *loadedTileIt;
	}

	auto queuedTileIt = std::find_if( m_Queue.begin(), m_Queue.end(), containsFn );
	if ( queuedTileIt != m_Queue.end() )
	{
		return *queuedTileIt;
	}
	else if ( m_LoadingTile && containsFn( m_LoadingTile ) )
	{
		return m_LoadingTile;
	}

	TileSharedPtr pTile = std::make_shared< Tile >( x, y, zoomLevel );
	m_Queue.push_back( pTile );
	return pTile;
}

int TileStreamer::TileStreamerThreadMain( TileStreamer* pTS )
{
	while ( pTS->m_RunThread )
	{
		pTS->m_AccessMutex.lock();
		if ( pTS->m_Queue.empty() == false )
		{ 
			pTS->m_LoadingTile = pTS->m_Queue.front();
			pTS->m_Queue.pop_front();
		}
		pTS->m_AccessMutex.unlock();

		if ( pTS->m_LoadingTile )
		{
			Tile& tile = *pTS->m_LoadingTile;
			if ( LoadFromFile( tile ) == false )
			{
				DownloadFromTileServer( tile );
				LoadFromFile( tile );
			}

			pTS->m_AccessMutex.lock();
			pTS->m_LoadedTiles.push_back( pTS->m_LoadingTile );
			pTS->m_AccessMutex.unlock();
			pTS->m_LoadingTile = nullptr;
		}

		if ( pTS->m_Queue.empty() )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
		}
	}

	return 0;
}

bool TileStreamer::LoadFromFile( Tile& tile )
{
	std::stringstream filename;
	filename << "textures/atlas/" << tile.ZoomLevel() << "/" << tile.X() << "_" << tile.Y() << ".png";
	if ( Filesystem::FileExists( filename.str() ) == false )
	{
		return false;
	}
	else
	{
		GLuint texture = TextureLoader::LoadTexture( filename.str() );
		if ( texture > 0 )
		{
			tile.AssignTexture( texture );
			return true;
		}
		else
		{
			return false;
		}
	}
}

static size_t WriteTileFileCallback( void* pBuffer, size_t size, size_t nmemb, void *pStream )
{
  return fwrite( pBuffer, size, nmemb, reinterpret_cast< FILE* >( pStream ) );
}

bool TileStreamer::DownloadFromTileServer( Tile& tile )
{
	CURL* pCurlHandle;
	std::stringstream url;
	url << "http://a.tile.stamen.com/toner/" << tile.ZoomLevel() << "/" << tile.X() << "/" << tile.Y() << ".png";
	std::stringstream filename;
	filename << "textures/atlas/" << tile.ZoomLevel() << "/" << tile.X() << "_" << tile.Y() << ".png";
	FILE* pTileFile = nullptr;

	pCurlHandle = curl_easy_init();
	curl_easy_setopt( pCurlHandle, CURLOPT_URL, url.str().c_str() );
	curl_easy_setopt( pCurlHandle, CURLOPT_NOPROGRESS, 1L );
	curl_easy_setopt( pCurlHandle, CURLOPT_WRITEFUNCTION, &WriteTileFileCallback );

#ifdef _WIN32
	fopen_s( &pTileFile, filename.str().c_str(), "wb" );
#else
	pTileFile = fopen( filename.str().c_str(), "wb" );
#endif
	bool result = false;
	if( pTileFile != nullptr ) 
	{
		curl_easy_setopt( pCurlHandle, CURLOPT_WRITEDATA, pTileFile );
		result = ( curl_easy_perform( pCurlHandle ) == CURLE_OK );
		fclose( pTileFile );
	}

	curl_easy_cleanup( pCurlHandle );
	return result;
}

void TileStreamer::CreateDirectories()
{
	for ( int zoomLevel = 0; zoomLevel < sMaxZoomLevels; ++zoomLevel )
	{
		std::stringstream path;
		path << "textures/atlas/" << zoomLevel;
		Filesystem::CreateDirectories( path.str() );
	}
}

}
