#pragma once

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <GL/gl.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class TextureLoader
{
public:
	static void Initialise();
	static void Update();
	static GLuint LoadTexture( const std::string& filename );
	static void UnloadTexture( GLuint texture );

private:
	static void ProcessQueuedLoads();
	static void ProcessQueuedUnloads();

	static std::thread::id m_MainThreadId;
	static std::queue< std::string > m_TextureLoadQueue;
	struct TextureLoadResult
	{
		std::string filename;
		GLuint texture;
	};
	static std::queue< TextureLoadResult > m_TextureLoadResultQueue;
	static std::mutex m_LoadMutex;
	static std::mutex m_ResultMutex;
	static std::queue< GLuint > m_TextureUnloadQueue;
	static std::mutex m_UnloadMutex;
};