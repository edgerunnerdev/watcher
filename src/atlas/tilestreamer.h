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
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <SDL_opengl.h>

#include "atlas/tile.h"

namespace Watcher
{

class TileStreamer
{
public:
	TileStreamer();
	~TileStreamer();

	// Gets a tile for a given zoom level. If a tile is "static", it will never be streamed out once loaded.
	TileSharedPtr Get(int x, int y, int zoomLevel, bool isStatic);

	static void ShowDebugUI(bool* pOpen);
	
private:
	static int TileStreamerThreadMain( TileStreamer* pTileRequester );
	static bool LoadFromFile( Tile& tile );
	static bool DownloadFromTileServer( Tile& tile ); 
	void CreateDirectories();

	struct TileStreamInfo 
	{
		TileStreamInfo() 
		{
			pTile = nullptr;
			isStatic = false;
		}

		TileSharedPtr pTile;
		bool isStatic;
	};
	using TileStreamInfoDeque = std::deque<TileStreamInfo>;

	std::mutex m_AccessMutex;
	TileStreamInfoDeque m_Queue;
	TileStreamInfoDeque m_LoadedTiles;
	TileStreamInfo m_LoadingTile;
	std::thread m_Thread;
	std::atomic_bool m_RunThread;
};

} // namespace Watcher
