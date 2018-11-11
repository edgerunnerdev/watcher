#include <SDL.h>
#include "imgui/imgui.h"

#include "atlas/atlas.h"
#include "atlas_grid.h"
#include "log.h"
#include "watcher_rep.h"
#include "watcher.h"

WatcherRep::WatcherRep( SDL_Window* pWindow ) :
m_pWindow( pWindow ),
m_CellSize( 128.0f )
{
	int windowWidth;
	int windowHeight;
	SDL_GetWindowSize( m_pWindow, &windowWidth, &windowHeight );
	m_pAtlasGrid = std::make_unique< AtlasGrid >( windowWidth, windowHeight, 16 );
	m_pAtlas = std::make_unique< Atlas::Atlas >( windowWidth, windowHeight );
}

WatcherRep::~WatcherRep()
{

}

void WatcherRep::ProcessEvent( const SDL_Event& event )
{
	if ( event.type == SDL_MOUSEMOTION )
	{
		const SDL_MouseMotionEvent* ev = reinterpret_cast< const SDL_MouseMotionEvent* >( &event );
		if ( ( ev->state & SDL_BUTTON_LMASK ) > 0 )
		{
			m_pAtlas->OnMouseDrag( ev->xrel, ev->yrel );
		}
	}
}

void WatcherRep::Update()
{
	static const float sBaseCellSize = 128.0f;
	m_CellSize = sBaseCellSize;

	ImGuiIO& io = ImGui::GetIO();
	if ( io.WantCaptureMouse == false )
	{

	}
}

void WatcherRep::Render()
{
	unsigned int flags = 0;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoSavedSettings;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGuiIO& io = ImGui::GetIO();
	ImTextureID my_tex_id = io.Fonts->TexID; 

	int windowWidth;
	int windowHeight;
	SDL_GetWindowSize( m_pWindow, &windowWidth, &windowHeight );
	ImVec2 windowSize = ImVec2( static_cast< float >( windowWidth ), static_cast< float >( windowHeight ) );

	ImGui::SetNextWindowPos( ImVec2( 0.0f, 0.0f ) );
	ImGui::SetNextWindowSize( windowSize );
	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImColor( 0, 0, 0, 0 ).Value );
	ImGui::Begin( "Watcher", nullptr, flags );

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	m_pAtlas->Render();

	ImGui::PopStyleColor();
	ImGui::End();

	GeoInfoVector geoInfos = g_pWatcher->GetGeoInfos();
	for ( const GeoInfo& geoInfo : geoInfos )
	{
		float locationX, locationY;
		m_pAtlas->GetScreenCoordinates( geoInfo.GetLongitude(), geoInfo.GetLatitude(), locationX, locationY );
		pDrawList->AddCircle( ImVec2( locationX, locationY ), 4.0f, ImColor( 255, 0, 0 ), 4 );
	}
}

void WatcherRep::AddToAtlas( float latitude, float longitude, int index )
{
	m_pAtlasGrid->Add( latitude, longitude, index );
}