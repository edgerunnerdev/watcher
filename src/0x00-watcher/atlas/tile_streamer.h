#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
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
	void RequestLoad( int x, int y, int zoomLevel );
	void GetLoadedTiles( int zoomLevel, TileVector& loadedTiles );
	
private:
	static int TileStreamerThreadMain( TileStreamer* pTileRequester );

	std::mutex m_AccessMutex;
	std::queue< Tile > m_Queue;
	TileVector m_LoadingTiles;
	TileVector m_LoadedTiles;
	std::thread m_Thread;
	std::atomic_bool m_RunThread;
};

}
