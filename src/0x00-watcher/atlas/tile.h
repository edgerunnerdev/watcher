#pragma once

#include <vector>

namespace Atlas
{

class Tile;
using TileVector = std::vector< Tile >;

class Tile
{
public:
	Tile();
	Tile( int x, int y, int zoomLevel );
	bool IsValid() const;
	int X() const;
	int Y() const;
	int ZoomLevel() const;

private:
	int m_X;
	int m_Y;
	int m_ZoomLevel;
};

}