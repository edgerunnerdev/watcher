#pragma once

#include <memory>
#include <vector>

namespace Atlas
{

class Tile;
using TileSharedPtr = std::shared_ptr< Tile >;
using TileVector = std::vector< TileSharedPtr >;

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