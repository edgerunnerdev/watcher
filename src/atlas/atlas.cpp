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

#include <algorithm>
#include <sstream>
#include <string>
#include <SDL.h>
#include "atlas/atlas.h"
#include "atlas/tilestreamer.h"
#include "imgui/imgui.h"
#include "log.h"

namespace Watcher
{

Atlas::Atlas( int windowWidth, int windowHeight ) :
m_MinimumZoomLevel( 0 ),
m_CurrentZoomLevel( 0 ),
m_MaxVisibleTilesX( 0 ),
m_MaxVisibleTilesY( 0 ),
m_OffsetX( 0 ),
m_OffsetY( 0 ),
m_WindowWidth( windowWidth ),
m_WindowHeight( windowHeight )
{
	m_pTileStreamer = std::make_unique< TileStreamer >();
	OnWindowSizeChanged( windowWidth, windowHeight );
}

Atlas::~Atlas()
{

}

void Atlas::ClampOffset()
{
	const int stride = static_cast< int >( pow( 2, m_CurrentZoomLevel ) );
	if ( m_OffsetX > 0 ) m_OffsetX = 0;
	if ( m_OffsetY > 0 ) m_OffsetY = 0;
	if ( -m_OffsetX > ( stride * sTileSize - m_WindowWidth ) ) m_OffsetX = -( stride * sTileSize - m_WindowWidth );
	if ( -m_OffsetY > ( stride * sTileSize - m_WindowHeight ) ) m_OffsetY = -( stride * sTileSize - m_WindowHeight );
}

void Atlas::OnMouseDrag( int deltaX, int deltaY )
{
	m_OffsetX += deltaX;
	m_OffsetY += deltaY;
	ClampOffset();
}

void Atlas::OnWindowSizeChanged( int windowWidth, int windowHeight )
{ 
	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;
	m_MaxVisibleTilesX = static_cast< int >( std::ceil( static_cast< float >( windowWidth ) / sTileSize ) ) + 1;
	m_MaxVisibleTilesY = static_cast< int >( std::ceil( static_cast< float >( windowHeight ) / sTileSize ) ) + 1;
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

	ClampOffset();
}

void Atlas::OnZoomIn()
{
	if ( m_CurrentZoomLevel + 1 < sMaxZoomLevels )
	{
		m_CurrentZoomLevel++;
		m_OffsetX = m_OffsetX * 2 - m_WindowWidth / 2;
		m_OffsetY = m_OffsetY * 2 - m_WindowHeight / 2;
		ClampOffset();
	}
}

void Atlas::OnZoomOut()
{
	if ( m_CurrentZoomLevel - 1 >= m_MinimumZoomLevel )
	{
		m_CurrentZoomLevel--;
		m_OffsetX = m_OffsetX / 2 + m_WindowWidth / 4;
		m_OffsetY = m_OffsetY / 2 + m_WindowHeight / 4;
		ClampOffset();
	}
}

void Atlas::CalculateVisibleTiles( TileVector& visibleTiles )
{
	// The minimum zoom level should always be marked as static, so it is never streamed out.
	// This helps with having a smoother transition when zooming back out.
	const bool isStatic = ( m_CurrentZoomLevel == m_MinimumZoomLevel );

	const int stride = static_cast< int >( pow( 2, m_CurrentZoomLevel ) );
	const int minX = std::max( 0, (int)( (float)-m_OffsetX / (float)sTileSize ) );
	const int maxX = std::min( minX + m_MaxVisibleTilesX, stride );
	const int minY = std::max( 0, (int)( (float)-m_OffsetY / (float)sTileSize ) );
	const int maxY = std::min( minY + m_MaxVisibleTilesY, stride );
	for ( int y = minY; y < maxY; ++y )
	{
		for ( int x = minX; x < maxX; ++x )
		{
			visibleTiles.push_back(m_pTileStreamer->Get(x, y, m_CurrentZoomLevel, isStatic));
		}
	}
}

void Atlas::Update(float delta)
{
	m_pTileStreamer->Update(delta);
}

void Atlas::Render()
{
	TileVector visibleTiles;
	visibleTiles.reserve( 32 );
	CalculateVisibleTiles( visibleTiles );

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	const ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
	for ( TileSharedPtr pTile : visibleTiles )
	{
		const int x = pTile->X();
		const int y = pTile->Y();
		ImVec2 p1( static_cast< float >( x * sTileSize + m_OffsetX + viewportPos.x ), static_cast< float >( y * sTileSize + m_OffsetY + viewportPos.y ) );
		ImVec2 p2( static_cast< float >( ( x + 1 ) * sTileSize + m_OffsetX + viewportPos.x ), static_cast< float >( ( y + 1 ) * sTileSize + m_OffsetY + viewportPos.y ) );

		if ( pTile->Texture() == 0 )
		{
			pDrawList->AddText( p1, ImColor( 1.0f, 1.0f, 1.0f ), "Loading..." );
		}
		else
		{
			pDrawList->AddImage(reinterpret_cast<ImTextureID>(uintptr_t(pTile->Texture())), p1, p2);
		}
	}
}

void Atlas::GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const
{
	// Uniform to Mercator projection, as per https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Resolution_and_Scale
	const int stride = static_cast< int >( pow( 2, m_CurrentZoomLevel ) );
	const float pi = static_cast< float >( M_PI );
	x = ( longitude + 180.0f ) / 360.0f * stride;
	y = ( 1.0f - logf( tanf( latitude * pi / 180.0f ) + 1.0f / cosf( latitude * pi / 180.0f ) ) / pi ) / 2.0f * stride;

	// To screenspace.
	const ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
	x = x * sTileSize + m_OffsetX + viewportPos.x;
	y = y * sTileSize + m_OffsetY + viewportPos.y;
}

} // namespace Watcher
