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
m_MinimumZoomLevel( 0 ),
m_CurrentZoomLevel( 0 ),
m_MaxVisibleTilesX( 0 ),
m_MaxVisibleTilesY( 0 ),
m_OffsetX( 0 ),
m_OffsetY( 0 )
{
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

	if ( m_OffsetX > 0 ) m_OffsetX = 0;
	if ( m_OffsetY > 0 ) m_OffsetY = 0;
}

void Atlas::OnWindowSizeChanged( int width, int height )
{ 
	m_MaxVisibleTilesX = static_cast< int >( std::ceilf( static_cast< float >( width ) / sTileSize ) ) + 1;
	m_MaxVisibleTilesY = static_cast< int >( std::ceilf( static_cast< float >( height ) / sTileSize ) ) + 1;
	const int maxAxisVisibleTiles = std::max( m_MaxVisibleTilesX, m_MaxVisibleTilesY );
	for ( int zoomLevel = 0; zoomLevel < sMaxZoomLevels; ++zoomLevel )
	{
		const int squareSize = static_cast< int >( pow( 2, zoomLevel ) );
		if ( maxAxisVisibleTiles <= squareSize )
		{
			m_MinimumZoomLevel = m_CurrentZoomLevel = zoomLevel;
			break;
		}
	}
}

void Atlas::OnZoomIn()
{
	m_CurrentZoomLevel = std::min( m_CurrentZoomLevel + 1, sMaxZoomLevels );
}

void Atlas::OnZoomOut()
{
	m_CurrentZoomLevel = std::max( m_MinimumZoomLevel, m_CurrentZoomLevel - 1 );
}

void Atlas::CalculateVisibleTiles( TileVector& visibleTiles )
{
	const int stride = static_cast< int >( pow( 2, m_CurrentZoomLevel ) );
	const int minX = std::max( 0, (int)( (float)-m_OffsetX / (float)sTileSize ) );
	const int maxX = std::min( minX + m_MaxVisibleTilesX, stride );
	const int minY = std::max( 0, (int)( (float)-m_OffsetY / (float)sTileSize ) );
	const int maxY = std::min( minY + m_MaxVisibleTilesY, stride );
	for ( int y = minY; y < maxY; ++y )
	{
		for ( int x = minX; x < maxX; ++x )
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
	for ( TileSharedPtr pTile : visibleTiles )
	{
		const int x = pTile->X();
		const int y = pTile->Y();
		ImVec2 p1( x * sTileSize + m_OffsetX, y * sTileSize + m_OffsetY );
		ImVec2 p2(  ( x + 1 ) * sTileSize + m_OffsetX, ( y + 1 ) * sTileSize + m_OffsetY );

		if ( pTile->Texture() == 0 )
		{
			pDrawList->AddText( p1, ImColor( 1.0f, 1.0f, 1.0f ), "Loading..." );
		}
		else
		{
			pDrawList->AddImage( reinterpret_cast< ImTextureID >( pTile->Texture() ), p1, p2 );
		}
		//pDrawList->AddRect( p1, p2, ImColor( 0.0f, 1.0f, 0.0f ) );
	}
}

void Atlas::GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const
{
	const int stride = static_cast< int >( pow( 2, m_CurrentZoomLevel ) );
	x = ( longitude + 180.0f ) / 360.0f * static_cast< float >( stride * sTileSize ) + m_OffsetX;
	y = ( 1.0f - ( latitude + 90.0f ) / 180.0f ) * static_cast< float >( stride * sTileSize ) + m_OffsetY;
}

}