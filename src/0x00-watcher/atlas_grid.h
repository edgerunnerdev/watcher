#include <vector>

#include "geo_info.h"

class AtlasGrid
{
public:
	AtlasGrid( int screenWidth, int screenHeight, int resolution );
	std::vector< int >* Pick( int x, int y ) const;
	void Add( float latitude, float longitude, int index );

private:
	std::vector< std::vector< std::vector< int >* > > m_Grid;
};