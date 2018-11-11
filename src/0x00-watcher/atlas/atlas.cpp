#include <algorithm>
#include <sstream>
#include <string>
#include <SDL.h>
#include "atlas/atlas.h"
#include "atlas/tile_streamer.h"
#include "imgui/imgui.h"
#include "log.h"

namespace Atlas
{

Atlas::Atlas( int windowWidth, int windowHeight ) :
m_NumTilesX( 8 ),
m_NumTilesY( 4 ),
m_TileResolution( 256 ),
m_MinimumZoomLevel( 0 ),
m_CurrentZoomLevel( 0 ),
m_OffsetX( 0 ),
m_OffsetY( 0 )
{
	SDL_assert( m_NumTilesX > 0 );
	SDL_assert( m_NumTilesY > 0 );
	SDL_assert( m_TileResolution >= 256 );

	m_pTileStreamer = std::make_unique< TileStreamer >();
	OnWindowSizeChanged( windowWidth, windowHeight );
}

Atlas::~Atlas()
{

}

void Atlas::OnMouseDrag( int deltaX, int deltaY )
{
	m_OffsetX += deltaX;
	m_OffsetY += deltaY;
}

void Atlas::OnWindowSizeChanged( int width, int height )
{ 
	const int maxView = std::max( width, height );
	const float maxAxisVisibleTiles = static_cast< float >( maxView ) / m_sTileSize;
	const int maxZoomLevels = 8;
	for ( int zoomLevel = 0; zoomLevel < maxZoomLevels; ++zoomLevel )
	{
		const int squareSize = static_cast< int >( pow( 2, zoomLevel ) );
		if ( maxAxisVisibleTiles <= squareSize )
		{
			m_MinimumZoomLevel = m_CurrentZoomLevel = zoomLevel;
			break;
		}
	}
}

void Atlas::CalculateVisibleTiles( TileVector& visibleTiles )
{
	// TODO: Calculate the proper visibility rectangle.
	int stride = static_cast< int >( pow( 2, m_CurrentZoomLevel ) );
	float tileSize = 256.0f;
	for ( int y = 0; y < stride; ++y )
	{
		for ( int x = 0; x < stride; ++x )
		{
			visibleTiles.push_back( m_pTileStreamer->Get( x, y, m_CurrentZoomLevel ) );
		}
	}
}

void Atlas::Render()
{
	TileVector visibleTiles;
	visibleTiles.reserve( 32 );
	CalculateVisibleTiles( visibleTiles );

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	const float tileSize = 256.0f;
	for ( TileSharedPtr pTile : visibleTiles )
	{
		const int x = pTile->X();
		const int y = pTile->Y();
		pDrawList->AddImage(
			reinterpret_cast< ImTextureID >( pTile->Texture() ), 
			ImVec2( x * tileSize + m_OffsetX, y * tileSize + m_OffsetY ),
			ImVec2( ( x + 1 ) * tileSize + m_OffsetX, ( y + 1 ) * tileSize + m_OffsetY )
		);
	}
}

void Atlas::GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const
{
	x = ( longitude + 180.0f ) / 360.0f * static_cast< float >( m_NumTilesX * m_TileResolution );
	y = ( 1.0f - ( latitude + 90.0f ) / 180.0f ) * static_cast< float >( m_NumTilesY * m_TileResolution );
}

}