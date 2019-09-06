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

#include "camerarep.h"

CameraRep::CameraRep(const Camera& camera)
{
	m_Camera = camera;
	m_Open = true;
	glGenTextures(1, &m_Texture);

	//std::vector<uint8_t> bytes2;
	//int numBytes = 512 * 512 * 3;
	//for (int i = 0; i < numBytes; ++i)
	//{
	//	bytes2.push_back(0xFF);
	//}

	//glBindTexture(GL_TEXTURE_2D, m_Texture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, &bytes2[0]);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

const Camera& CameraRep::GetCamera() const
{
	return m_Camera;
}

bool CameraRep::IsOpen() const
{
	return m_Open;
}

void CameraRep::Close()
{
	m_Open = false;
	glDeleteTextures(1, &m_Texture);
}

GLuint CameraRep::GetTexture() const
{
	return m_Texture;
}