#pragma once

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <GL/gl.h>

#include "atlas/tile.h"

namespace Atlas
{

class Atlas;
class TileStreamer;
using AtlasUniquePtr = std::unique_ptr< Atlas >;

static const int sTileSize = 256;
static const int sMaxZoomLevels = 8;

class Atlas
{
public:
	Atlas( int windowWidth, int windowHeight );
	~Atlas();

	void Render();
	void GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const;

	void OnWindowSizeChanged( int width, int height );
	void OnMouseDrag( int deltaX, int deltaY );
	void OnZoomIn();
	void OnZoomOut();

private:
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

}