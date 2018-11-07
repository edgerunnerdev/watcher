#include <SDL.h>
#include "imgui/imgui.h"

#include "atlas/atlas.h"
#include "atlas_grid.h"
#include "log.h"
#include "watcher_rep.h"
#include "watcher.h"

WatcherRep::WatcherRep( SDL_Window* pWindow ) :
m_pWindow( pWindow ),
m_BackgroundTexture( GL_INVALID_VALUE ),
m_CellSize( 128.0f )
{
	LoadTextures();

	int windowWidth;
	int windowHeight;
	SDL_GetWindowSize( m_pWindow, &windowWidth, &windowHeight );
	m_pAtlasGrid = std::make_unique< AtlasGrid >( windowWidth, windowHeight, 16 );
	m_pAtlas = std::make_unique< Atlas >( windowWidth, windowHeight );

	for ( GLuint x = 0u; x < m_BackgroundTextures.size(); ++x )
	{
		for ( GLuint y = 0; y < m_BackgroundTextures[ x ].size(); ++y )
		{
			m_BackgroundTextures[ x ][ y ] = 0u;
		}
	}
}

WatcherRep::~WatcherRep()
{

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
	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImColor(0, 0, 0, 0).Value );
	ImGui::Begin( "Watcher", nullptr, flags );

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	m_pAtlas->Render();

	ImGui::PopStyleColor();
	ImGui::End();

	const GeoInfoVector& geoInfos = g_pWatcher->GetGeoInfos();
	for ( const GeoInfo& geoInfo : geoInfos )
	{
		float locationX, locationY;
		m_pAtlas->GetScreenCoordinates( geoInfo.GetLongitude(), geoInfo.GetLatitude(), locationX, locationY );
		pDrawList->AddCircle( ImVec2( locationX, locationY ), 4.0f, ImColor( 255, 0, 0 ), 4 );
	}
}

void WatcherRep::LoadTextures()
{
	m_BackgroundTexture = LoadTexture( "textures/Earth_Diffuse_small.bmp" );
}

GLuint WatcherRep::LoadTexture( const std::string& filename )
{
	SDL_Surface* pSurface = SDL_LoadBMP( filename.c_str() );
	SDL_assert( pSurface != nullptr );
	if ( pSurface == nullptr )
	{
		return GL_INVALID_VALUE;
	}

	GLuint tex;
	glGenTextures( 1, &tex );
	glBindTexture( GL_TEXTURE_2D, tex );
	int mode = ( pSurface->format->BytesPerPixel == 4 ) ? GL_RGBA : GL_RGB;

	glTexImage2D( GL_TEXTURE_2D, 0, mode, pSurface->w, pSurface->h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pSurface->pixels );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	return tex;
}

void WatcherRep::AddToAtlas( float latitude, float longitude, int index )
{
	m_pAtlasGrid->Add( latitude, longitude, index );
}