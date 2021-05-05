///////////////////////////////////////////////////////////////////////////////
// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <SDL_opengl.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace Watcher
{

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

} // namespace Watcher
