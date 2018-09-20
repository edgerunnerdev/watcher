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
#include <vector>

#include <GL/gl.h>

class Atlas;
using AtlasUniquePtr = std::unique_ptr< Atlas >;

class Atlas
{
public:
	Atlas(int numTilesX, int numTilesY, int tileResolution);
	~Atlas();

	void Render();

private:
	void LoadTextures();
	GLuint LoadTexture( const std::string& filename );
	GLuint GetTileTexture( int x, int y ) const;

	int m_NumTilesX;
	int m_NumTilesY;
	int m_TileResolution;
	std::vector< GLuint > m_LowResTextures;
	std::vector< GLuint > m_HighResTextures;
};