#pragma once

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <string>

#include <GL/GL.h>

struct SDL_Surface;
struct SDL_Window;

class WatcherRep
{
public:
	WatcherRep( SDL_Window* pWindow );
	~WatcherRep();

	void Render();

private:
	void LoadTextures();
	GLuint LoadTexture( const std::string& filename );	

	SDL_Window* m_pWindow;
	GLuint m_BackgroundTexture;
};