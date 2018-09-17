#include <SDL.h>
#include "atlas_grid.h"

AtlasGrid::AtlasGrid( int screenWidth, int screenHeight, int resolution )
{
	SDL_assert( screenWidth > 0 );
	SDL_assert( screenHeight > 0 );
	SDL_assert( resolution > 0 );

	int maxX = screenWidth / resolution;
	int maxY = screenHeight / resolution;

	m_Grid.resize( maxX );
	for ( int x = 0; x < maxX; x++ )
	{
		m_Grid[ x ].resize( maxY );

		for ( int y = 0; y < maxY; y++ )
		{
			m_Grid[ x ][ y ] = nullptr;
		}
	}
}

std::vector< int >* AtlasGrid::Pick( int x, int y ) const
{
	return m_Grid[ x ][ y ];
}

void AtlasGrid::Add( float latitude, float longitude, int index )
{
	int x = ( longitude + 180.0f ) / 360.0f * static_cast< float >( m_Grid.size() );
	int y = ( 1.0f - ( latitude + 90.0f ) / 180.0f ) * static_cast< float >( m_Grid[0].size() );
	std::vector< int >* v = m_Grid[ x ][ y ];
	if ( v == nullptr )
	{
		m_Grid[ x ][ y ] = new std::vector< int >();
		m_Grid[ x ][ y ]->push_back( index );
	}
	else
	{
		v->push_back( index );
	}
}
