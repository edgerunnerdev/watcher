#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <GL/gl.h>

#include "atlas/tile.h"

namespace Atlas
{

class TileStreamer
{
public:
	TileStreamer();
	~TileStreamer();
	TileSharedPtr Get( int x, int y, int zoomLevel );
	
private:
	static int TileStreamerThreadMain( TileStreamer* pTileRequester );
	static bool LoadFromFile( Tile& tile );
	static bool DownloadFromTileServer( Tile& tile ); 
	void CreateDirectories();

	std::mutex m_AccessMutex;
	std::deque< TileSharedPtr > m_Queue;
	TileVector m_LoadedTiles;
	TileSharedPtr m_LoadingTile;
	std::thread m_Thread;
	std::atomic_bool m_RunThread;
};

}
