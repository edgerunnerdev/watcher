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

#include "atlas/tile.h"
#include "textureloader.h"

namespace Watcher
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

} // namespace Watcher
