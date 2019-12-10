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

#include "imgui/imgui.h"
#include "camerarep.h"

CameraRep::CameraRep(const Camera& camera)
{
	m_Camera = camera;
	m_Open = true;
	m_WindowWidth = 480.0f;
	m_WindowHeight = 320.0f;
	m_TextureWidth = 0;
	m_TextureHeight = 0;
	glGenTextures(1, &m_Texture);
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

void CameraRep::Render()
{
	if (!m_Open)
	{
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(m_WindowWidth, m_WindowHeight), ImGuiCond_Always);
	if (ImGui::Begin(m_Camera.GetURL().c_str(), &m_Open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize))
	{
		ImVec2 windowPos = ImGui::GetWindowPos();

		if (m_TextureWidth == 0 && m_TextureHeight == 0)
		{
			glBindTexture(GL_TEXTURE_2D, m_Texture);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_TextureWidth);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_TextureHeight);
			if (m_TextureWidth > 0 && m_TextureHeight > 0)
			{
				m_WindowWidth = static_cast<float>(m_TextureWidth);
				m_WindowHeight = static_cast<float>(m_TextureHeight);
				m_Camera.SetState(Camera::State::StreamAvailable);
			}
		}

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImTextureID cameraTexture = reinterpret_cast<ImTextureID>(GetTexture());
		pDrawList->AddImage(cameraTexture, windowPos, ImVec2(windowPos.x + m_WindowWidth, windowPos.y + m_WindowHeight));

		GeolocationData* pGeo = m_Camera.GetGeolocationData();
		ImGui::Text("%s, %s, %s", pGeo->GetCity().c_str(), pGeo->GetRegion().c_str(), pGeo->GetCountry().c_str());
		ImGui::Text("%s", pGeo->GetIPAddress().GetHostAsString().c_str());
	}

	ImGui::End();
}