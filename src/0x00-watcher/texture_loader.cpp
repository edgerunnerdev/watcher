#include <SDL.h>
#include <SDL_image.h>
#include "log.h"
#include "texture_loader.h"

std::thread::id TextureLoader::m_MainThreadId;
std::queue< std::string > TextureLoader::m_TextureLoadQueue;
std::queue< TextureLoader::TextureLoadResult > TextureLoader::m_TextureLoadResultQueue;
std::mutex TextureLoader::m_LoadMutex;
std::mutex TextureLoader::m_ResultMutex;

void TextureLoader::Initialise()
{
	m_MainThreadId = std::this_thread::get_id();
}

void TextureLoader::Update()
{
	std::scoped_lock< std::mutex > loadLock( m_LoadMutex );
	while ( m_TextureLoadQueue.empty() == false )
	{
		GLuint texture = LoadTexture( m_TextureLoadQueue.front() );
		{
			std::scoped_lock< std::mutex > resultLock( m_ResultMutex );
			TextureLoadResult result;
			result.filename = m_TextureLoadQueue.front();
			result.texture = texture;
			m_TextureLoadResultQueue.push( result );
			m_TextureLoadQueue.pop();
		}
	}
}

GLuint TextureLoader::LoadTexture( const std::string& filename )
{
	if ( std::this_thread::get_id() != m_MainThreadId )
	{
		{
			std::scoped_lock< std::mutex > lock( m_LoadMutex );
			m_TextureLoadQueue.push( filename );
		}

		while ( 1 )
		{
			std::scoped_lock< std::mutex > lock( m_ResultMutex );
			if ( m_TextureLoadResultQueue.empty() == false && m_TextureLoadResultQueue.front().filename == filename )
			{
				GLuint texture = m_TextureLoadResultQueue.front().texture;
				m_TextureLoadResultQueue.pop();
				return texture;
			}
		}
	}

	SDL_Surface* pSurface = IMG_Load( filename.c_str() );
	SDL_assert( pSurface != nullptr );
	if ( pSurface == nullptr )
	{
 		Log::Error( "WatcherRep::LoadTexture error: %s", IMG_GetError() );
		return 0;
	}

	GLuint tex;
	glGenTextures( 1, &tex );
	GLenum err = glGetError();
	glBindTexture( GL_TEXTURE_2D, tex );
	int bpp = pSurface->format->BytesPerPixel;
	int internalFormat = ( bpp == 4 ) ? GL_RGBA : GL_RGB;
	int format = ( bpp == 4 ) ? GL_BGRA_EXT : GL_BGR_EXT;

	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, pSurface->w, pSurface->h, 0, format, GL_UNSIGNED_BYTE, pSurface->pixels );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	return tex;
}
