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

#include "camera.h"

struct SDL_Surface;
struct SDL_Window;

namespace Atlas
{
class Atlas;
using AtlasUniquePtr = std::unique_ptr< Atlas >;
}

class WatcherRep
{
public:
	WatcherRep( SDL_Window* pWindow );
	~WatcherRep();

	void ProcessEvent( const SDL_Event& event );
	void Update();
	void Render();

private:
	void SetUserInterfaceStyle();
	CameraVector GetHoveredCameras();

	SDL_Window* m_pWindow;
	Atlas::AtlasUniquePtr m_pAtlas;
	float m_CellSize;
	GLuint m_PinTexture;
};