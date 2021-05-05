///////////////////////////////////////////////////////////////////////////////
// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

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

namespace Watcher
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
	TileDeque m_Queue;
	TileDeque m_LoadedTiles;
	TileSharedPtr m_LoadingTile;
	std::thread m_Thread;
	std::atomic_bool m_RunThread;
};

} // namespace Watcher
