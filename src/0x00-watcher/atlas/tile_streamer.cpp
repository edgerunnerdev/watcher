#pragma once

#include <algorithm>
#include <chrono>
#include <sstream>

#include <curl/curl.h>

#include "atlas/tile_streamer.h"
#include "atlas/tile.h"
#include "log.h"

namespace Atlas
{

TileStreamer::TileStreamer()
{
	m_RunThread = true;
	m_Thread = std::thread( &TileStreamer::TileStreamerThreadMain, this );
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
			Log::Info( "Loading tile %d / %d, zoom level %d", 
				pTS->m_LoadingTile->X(), 
				pTS->m_LoadingTile->Y(), 
				pTS->m_LoadingTile->ZoomLevel() 
			);

			Tile& tile = *pTS->m_LoadingTile;
			if ( LoadFromFile( tile ) == false )
			{
				DownloadFromTileServer( tile );
				LoadFromFile( tile );
			}

			Log::Info( "Loaded tile %d / %d, zoom level %d", 
				pTS->m_LoadingTile->X(), 
				pTS->m_LoadingTile->Y(), 
				pTS->m_LoadingTile->ZoomLevel() 
			);
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
	return false;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

bool TileStreamer::DownloadFromTileServer( Tile& tile )
{
	CURL* curl_handle;
	std::stringstream url;
	url << "http://a.tile.stamen.com/toner/" << tile.ZoomLevel() << "/" << tile.X() << "/" << tile.Y() << ".png";
	std::stringstream filename;
	filename << "textures/atlas/" << tile.ZoomLevel() << "/" << tile.X() << "_" << tile.Y() << ".png";
	FILE* pagefile;

	/* init the curl session */ 
	curl_handle = curl_easy_init();
 
	/* set URL to get here */ 
	curl_easy_setopt( curl_handle, CURLOPT_URL, url.str().c_str() );
 
	/* Switch on full protocol/debug output while testing */ 
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
 
	/* disable progress meter, set to 0L to enable and disable debug output */ 
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
 
	/* send all data to this function  */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
 
	/* open the file */ 
	fopen_s( &pagefile, filename.str().c_str(), "wb" );
	if( pagefile ) 
	{
		/* write the page body to this file handle */ 
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
 
		/* get it! */ 
		curl_easy_perform(curl_handle);
 
		/* close the header file */ 
		fclose(pagefile);
	}
 
	/* cleanup curl stuff */ 
	curl_easy_cleanup(curl_handle);

	return true;
}

}
