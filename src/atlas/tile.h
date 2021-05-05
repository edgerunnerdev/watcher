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

#pragma once

#include <deque>
#include <memory>
#include <vector>

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <GL/gl.h>

namespace Watcher
{

class Tile;
using TileSharedPtr = std::shared_ptr< Tile >;
using TileVector = std::vector< TileSharedPtr >;
using TileDeque = std::deque< TileSharedPtr >;

class Tile
{
public:
	Tile();
	Tile( int x, int y, int zoomLevel );
	~Tile();
	int X() const;
	int Y() const;
	int ZoomLevel() const;

	void AssignTexture( GLuint texture );
	GLuint Texture() const;

private:
	int m_X;
	int m_Y;
	int m_ZoomLevel;
	GLuint m_Texture;
};

} // namespace Watcher
