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

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <SDL_opengl.h>

#include "atlas/tile.h"

namespace Watcher
{

class Atlas;
class TileStreamer;
using AtlasUniquePtr = std::unique_ptr< Atlas >;

static const int sTileSize = 256;
static const int sMaxZoomLevels = 11;

class Atlas
{
public:
	Atlas( int windowWidth, int windowHeight );
	~Atlas();

	void Update(float delta);
	void Render();
	void GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const;

	void OnWindowSizeChanged( int width, int height );
	void OnMouseDrag( int deltaX, int deltaY );
	void OnZoomIn();
	void OnZoomOut();

private:
	void ClampOffset();
	void CalculateVisibleTiles( TileVector& visibleTiles );

	using TileTextureIdVector = std::vector< GLuint >;
	using TileMaps = std::vector< TileTextureIdVector >;
	TileMaps m_TileMaps;
	int m_MinimumZoomLevel;
	int m_CurrentZoomLevel;
	int m_MaxVisibleTilesX;
	int m_MaxVisibleTilesY;
	int m_OffsetX;
	int m_OffsetY;
	int m_WindowWidth;
	int m_WindowHeight;

	std::vector< int > m_TilesToDraw;
	std::unique_ptr< TileStreamer > m_pTileStreamer;
};

} // namespace Watcher
