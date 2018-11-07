#pragma once

#include <algorithm>
#include <chrono>

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

Tile TileStreamer::Get( int x, int y, int zoomLevel )
{
	std::lock_guard< std::mutex > lock( m_AccessMutex );
	for ( Tile& tile : m_LoadedTiles )
	{
		if ( tile.X() == x && tile.Y() == y && tile.ZoomLevel() == zoomLevel )
		{
			return tile;
		}
	}

	m_Queue.emplace( x, y, zoomLevel );
	return Tile( x, y, zoomLevel );
}

int TileStreamer::TileStreamerThreadMain( TileStreamer* pTileRequester )
{
	while ( pTileRequester->m_RunThread )
	{
		Tile tile;
		{
			std::lock_guard< std::mutex > lock( pTileRequester->m_AccessMutex );
			if ( pTileRequester->m_Queue.empty() == false )
			{
				tile = pTileRequester->m_Queue.front();
				pTileRequester->m_Queue.pop();
			}
		}

		if ( tile.IsValid() )
		{
			Log::Info( "Processed tile %d / %d, zoom level %d", tile.X(), tile.Y(), tile.ZoomLevel() );
			std::lock_guard< std::mutex > lock( pTileRequester->m_AccessMutex );
			pTileRequester->m_LoadedTiles.push_back( tile );
		}

		if ( pTileRequester->m_Queue.empty() )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
		}
	}

	return 0;
}

}
