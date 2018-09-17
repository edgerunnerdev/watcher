#pragma once

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <array>
#include <memory>
#include <string>

#include <GL/gl.h>

struct SDL_Surface;
struct SDL_Window;
class Atlas;

using AtlasGridUniquePtr = std::unique_ptr< AtlasGrid >;
using AtlasUniquePtr = std::unique_ptr< Atlas >;

class WatcherRep
{
public:
	WatcherRep( SDL_Window* pWindow );
	~WatcherRep();

	void Update();
	void Render();

	void AddToAtlas( float latitude, float longitude, int index );

private:
	void LoadTextures();
	GLuint LoadTexture( const std::string& filename );	

	SDL_Window* m_pWindow;
	GLuint m_BackgroundTexture;
	AtlasUniquePtr m_pAtlas;
	AtlasGridUniquePtr m_pAtlasGrid;
	std::array< std::array< GLuint, 4 >, 8 > m_BackgroundTextures;
	float m_CellSize;
};