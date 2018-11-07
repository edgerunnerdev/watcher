#include <sstream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include "atlas/atlas.h"
#include "atlas/tile_streamer.h"
#include "imgui/imgui.h"

namespace Atlas
{

Atlas::Atlas( int windowWidth, int windowHeight ) :
m_NumTilesX( 8 ),
m_NumTilesY( 4 ),
m_TileResolution( 256 ),
m_LowResTextures( 8 * 4 ),
m_HighResTextures( 8 * 4 ),
m_MinimumZoomLevel( 0 ),
m_CurrentZoomLevel( 0 )
{
	SDL_assert( m_NumTilesX > 0 );
	SDL_assert( m_NumTilesY > 0 );
	SDL_assert( m_TileResolution >= 256 );

	m_pTileStreamer = std::make_unique< TileStreamer >();

	InitialiseTileMaps();
	OnWindowSizeChanged( windowWidth, windowHeight );

	LoadTextures();
}

Atlas::~Atlas()
{

}

void Atlas::InitialiseTileMaps()
{
	const int maxZoomLevels = 8;
	m_TileMaps.resize( maxZoomLevels );
	for ( int zoomLevel = 0; zoomLevel < maxZoomLevels; ++zoomLevel )
	{
		const int numTiles = static_cast< int >( pow( 4, zoomLevel ) );
		m_TileMaps[ zoomLevel ].resize( numTiles, 0 );
	}
}

void Atlas::OnWindowSizeChanged( int width, int height )
{ 
	const int maxView = max( width, height );
	const float maxAxisVisibleTiles = static_cast< float >( maxView ) / m_sTileSize;
	const int maxZoomLevels = static_cast< int >( m_TileMaps.size() );
	for ( int zoomLevel = 0; zoomLevel < maxZoomLevels; ++zoomLevel )
	{
		const int squareSize = static_cast< int >( sqrt( m_TileMaps[ zoomLevel ].size() ) );
		if ( maxAxisVisibleTiles <= squareSize )
		{
			m_MinimumZoomLevel = m_CurrentZoomLevel = zoomLevel;
			break;
		}
	}
	
	// TEMP
	int numTiles = m_TileMaps[ m_CurrentZoomLevel ].size();
	for ( int i = 0; i < numTiles; ++i )
	{
		m_TilesToDraw.push_back( i );
	}
}

void Atlas::CalculateVisibleTiles( TileVector& visibleTiles )
{
	int stride = static_cast< int >( sqrt( m_TileMaps[ m_CurrentZoomLevel ].size() ) );
	float tileSize = 256.0f;
	for ( int tileId : m_TilesToDraw )
	{
		int x = tileId % stride;
		int y = tileId / stride;

		m_pTileStreamer->RequestLoad( x, y, m_CurrentZoomLevel );
	}

	m_pTileStreamer->GetLoadedTiles( m_CurrentZoomLevel, visibleTiles );
}

void Atlas::Render()
{
	TileVector visibleTiles;
	visibleTiles.reserve( 32 );
	CalculateVisibleTiles( visibleTiles );

	//float zoom = 1.0f;
	//float tileSize = m_TileResolution * zoom;
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	//for ( int x = 0u; x < m_NumTilesX; ++x )
	//{
	//	for ( int y = 0; y < m_NumTilesY; ++y )
	//	{
	//		GLuint texture = GetTileTexture( x, y );
	//		if ( texture != 0u )
	//		{
	//			pDrawList->AddImage( 
	//				reinterpret_cast< ImTextureID >( texture ), 
	//				ImVec2( tileSize * (float)x, tileSize * (float)y ), 
	//				ImVec2( tileSize * (float)( x + 1u ), tileSize * (float)( y + 1u ) )
	//			);
	//		}
	//	}
	//}

	float tileSize = 256.0f;
	for ( Tile& tile : visibleTiles )
	{
		int x = tile.X();
		int y = tile.Y();

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

// Returns the texture for a given tile coordinate, prefering the high resolution version if loaded.
GLuint Atlas::GetTileTexture( int x, int y ) const
{
	int idx = x * m_NumTilesY + y;
	return m_HighResTextures.at( idx ) == 0 ? m_LowResTextures[ idx ] : m_HighResTextures[ idx ];
}

void Atlas::LoadTextures()
{
	int textureIndex = 0;
	for ( int x = 0u; x < m_NumTilesX; ++x )
	{
		for ( int y = 0u; y < m_NumTilesY; ++y )
		{
			std::stringstream filename;
			filename << "textures/earth_small_" << textureIndex + 1 << ".png";
			GLuint texture = LoadTexture( filename.str() );
			m_LowResTextures[ textureIndex++ ] = texture;
		}
	}
}

GLuint Atlas::LoadTexture( const std::string& filename )
{
	SDL_Surface* pSurface = IMG_Load( filename.c_str() );
	SDL_assert( pSurface != nullptr );
	if ( pSurface == nullptr )
	{
 		printf("Atlas::LoadTexture error: %s\n", IMG_GetError());
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