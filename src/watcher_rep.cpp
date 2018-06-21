#include <SDL.h>
#include "imgui/imgui.h"

#include "watcher_rep.h"
#include "watcher.h"

WatcherRep::WatcherRep( SDL_Window* pWindow ) :
m_pWindow( pWindow ),
m_BackgroundTexture( GL_INVALID_VALUE )
{
	LoadTextures();
}

WatcherRep::~WatcherRep()
{

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
	//pDrawList->AddQuadFilled( ImVec2( 0, 0 ), ImVec2( 512, 0 ), ImVec2( 512, 512 ), ImVec2( 0, 512 ), ImColor(255, 0, 0));
	pDrawList->AddImage( (ImTextureID)m_BackgroundTexture, ImVec2(0,0), windowSize );
	ImGui::PopStyleColor();
	ImGui::End();

	// TODO: replace interface with longitude / latitude
	const GeoInfoVector& geoInfos = g_pWatcher->GetGeoInfos();
	for ( const GeoInfo& geoInfo : geoInfos )
	{
		float locationX, locationY;
		locationX = ( geoInfo.GetLongitude() + 180.0f ) / 360.0f * static_cast< float >( windowWidth );
		locationY = ( 1.0f - ( geoInfo.GetLatitude() + 90.0f ) / 180.0f ) * static_cast< float >( windowHeight );
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
