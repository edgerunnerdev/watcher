#include "atlas/tile.h"

namespace Atlas
{

Tile::Tile() :
m_X( -1 ),
m_Y( -1 ),
m_ZoomLevel( -1 )
{

}

Tile::Tile( int x, int y, int zoomLevel ) :
m_X( x ),
m_Y( y ),
m_ZoomLevel( zoomLevel )
{

}

bool Tile::IsValid() const
{
	return ( m_X >= 0 && m_Y >= 0 && m_ZoomLevel >= 0 );
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

}