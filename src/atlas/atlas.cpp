#include <SDL.h>
#include <SDL_image.h>
#include "atlas/atlas.h"
#include "imgui/imgui.h"

Atlas::Atlas(int numTilesX, int numTilesY, int tileResolution) :
m_NumTilesX( numTilesX ),
m_NumTilesY( numTilesY ),
m_TileResolution( tileResolution )
{
	SDL_assert( m_NumTilesX > 0 );
	SDL_assert( m_NumTilesY > 0 );
	SDL_assert( m_TileResolution >= 256 );
}

Atlas::~Atlas()
{

}

void Atlas::Render()
{
	float zoom = 1.0f;
	float tileSize = m_TileResolution * zoom;
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	for ( int x = 0u; x < m_NumTilesX; ++x )
	{
		for ( int y = 0; y < m_NumTilesY; ++y )
		{
			pDrawList->AddRectFilled(
				ImVec2( tileSize * (float)x, tileSize * (float)y ),
				ImVec2( tileSize * (float)( x + 1u ), tileSize * (float)( y + 1u ) ),
				( ( x + y ) % 2 == 0 ) ? ImColor( 1.0f, 0.0f, 0.0f, 0.5f ) : ImColor( 0.0f, 0.0f, 1.0f, 0.5f ) 
			);
		}
	}
}