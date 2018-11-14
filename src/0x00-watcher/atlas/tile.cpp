#include "atlas/tile.h"
#include "texture_loader.h"

namespace Atlas
{

Tile::Tile() :
m_X( -1 ),
m_Y( -1 ),
m_ZoomLevel( -1 ),
m_Texture( 0 )
{

}

Tile::Tile( int x, int y, int zoomLevel ) :
m_X( x ),
m_Y( y ),
m_ZoomLevel( zoomLevel ),
m_Texture( 0 )
{

}

Tile::~Tile()
{
	if ( m_Texture != 0 )
	{
		TextureLoader::UnloadTexture( m_Texture );
	}
}

int Tile::X() const
{
	return m_X;
}

int Tile::Y() const
{
	return m_Y;
}

int Tile::ZoomLevel() const
{
	return m_ZoomLevel;
}

GLuint Tile::Texture() const
{
	return m_Texture;
}

void Tile::AssignTexture( GLuint texture )
{
	m_Texture = texture;
}

}