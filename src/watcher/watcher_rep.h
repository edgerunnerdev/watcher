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

namespace Atlas
{
class Atlas;
using AtlasUniquePtr = std::unique_ptr< Atlas >;
}

class AtlasGrid;
using AtlasGridUniquePtr = std::unique_ptr< AtlasGrid >;

class WatcherRep
{
public:
	WatcherRep( SDL_Window* pWindow );
	~WatcherRep();

	void ProcessEvent( const SDL_Event& event );
	void Update();
	void Render();

	void AddToAtlas( float latitude, float longitude, int index );

private:
	SDL_Window* m_pWindow;
	Atlas::AtlasUniquePtr m_pAtlas;
	AtlasGridUniquePtr m_pAtlasGrid;
	float m_CellSize;
};