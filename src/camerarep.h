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

#pragma once

#include <SDL_opengl.h>

#include "codecs/stream.h"
#include "camera.h"

namespace Watcher
{

class CameraRep
{
public:
	CameraRep(CameraWeakPtr cameraWeakPtr);

	CameraWeakPtr GetCameraWeakPtr() const;
	bool IsOpen() const;
	void Close();

    void Update();
	void Render();

	GLuint GetTexture() const;

private:
	static void ConstrainedRatio(ImGuiSizeCallbackData* pData);

	CameraWeakPtr m_CameraWeakPtr;
	GLuint m_Texture;
	GLint m_TextureWidth;
	GLint m_TextureHeight;
	float m_WindowWidth;
	float m_WindowHeight;
	bool m_Open;
    StreamSharedPtr m_pStream;
};

} // namespace Watcher
