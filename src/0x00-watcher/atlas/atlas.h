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

static const int sMaxZoomLevels = 8;

class Atlas
{
public:
	Atlas( int windowWidth, int windowHeight );
	~Atlas();

	void Render();
	void GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const;

	void OnWindowSizeChanged( int width, int height );

private:
	void CalculateVisibleTiles( TileVector& visibleTiles );

	static const int m_sTileSize = 256;

	int m_NumTilesX;
	int m_NumTilesY;
	int m_TileResolution;

	using TileTextureIdVector = std::vector< GLuint >;
	using TileMaps = std::vector< TileTextureIdVector >;
	TileMaps m_TileMaps;
	int m_MinimumZoomLevel;
	int m_CurrentZoomLevel;

	std::vector< int > m_TilesToDraw;
	std::unique_ptr< TileStreamer > m_pTileStreamer;
};

}