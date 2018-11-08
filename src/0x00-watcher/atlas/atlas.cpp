#include <sstream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
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
m_CurrentZoomLevel( 0 )
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

void Atlas::OnWindowSizeChanged( int width, int height )
{ 
	const int maxView = max( width, height );
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
	//		GLuint texture = GetTileTexture( x, y );
	//		if ( texture != 0u )
	//		{
	//			pDrawList->AddImage( 
	//				reinterpret_cast< ImTextureID >( texture ), 
	//				ImVec2( tileSize * (float)x, tileSize * (float)y ), 
	//				ImVec2( tileSize * (float)( x + 1u ), tileSize * (float)( y + 1u ) )
	//			);
	//		}

	float tileSize = 256.0f;
	for ( TileSharedPtr pTile : visibleTiles )
	{
		int x = pTile->X();
		int y = pTile->Y();

		bool useAltColor = ( x % 2 > 0 );
		if ( y % 2 > 0 )
		{
			useAltColor = !useAltColor;
		}
		pDrawList->AddRectFilled(
			ImVec2( x * tileSize, y * tileSize ),
			ImVec2( ( x + 1 ) * tileSize, ( y + 1 ) * tileSize ),
			useAltColor ? ImColor( 1.0f, 0.0f, 0.0f ) : ImColor( 0.0f, 1.0f, 0.0f ) );
	}
}

void Atlas::GetScreenCoordinates( float longitude, float latitude, float& x, float& y ) const
{
	x = ( longitude + 180.0f ) / 360.0f * static_cast< float >( m_NumTilesX * m_TileResolution );
	y = ( 1.0f - ( latitude + 90.0f ) / 180.0f ) * static_cast< float >( m_NumTilesY * m_TileResolution );
}

GLuint Atlas::LoadTexture( const std::string& filename )
{
	SDL_Surface* pSurface = IMG_Load( filename.c_str() );
	SDL_assert( pSurface != nullptr );
	if ( pSurface == nullptr )
	{
 		Log::Error( "Atlas::LoadTexture error: %s", IMG_GetError() );
		return 0;
	}

	GLuint tex;
	glGenTextures( 1, &tex );
	glBindTexture( GL_TEXTURE_2D, tex );
	int mode = ( pSurface->format->BytesPerPixel == 4 ) ? GL_RGBA : GL_RGB;

	glTexImage2D( GL_TEXTURE_2D, 0, mode, pSurface->w, pSurface->h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pSurface->pixels );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	return tex;
}

}