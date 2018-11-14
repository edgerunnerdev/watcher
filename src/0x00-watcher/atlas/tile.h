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

namespace Atlas
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

}