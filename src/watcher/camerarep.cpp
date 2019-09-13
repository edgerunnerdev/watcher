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
	m_Width = 480;
	m_Height = 320;
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

	ImGui::SetNextWindowSize(ImVec2(m_Width, m_Height), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(m_Camera.GetURL().c_str(), &m_Open, ImGuiWindowFlags_NoSavedSettings))
	{
		ImVec2 windowPos = ImGui::GetWindowPos();

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImTextureID cameraTexture = reinterpret_cast<ImTextureID>(GetTexture());
		pDrawList->AddImage(cameraTexture, windowPos, ImVec2(windowPos.x + m_Width, windowPos.y + m_Height));

		GeolocationData* pGeo = m_Camera.GetGeolocationData();
		ImGui::Text("%s, %s, %s (%s)", 
			pGeo->GetCity().c_str(),
			pGeo->GetRegion().c_str(),
			pGeo->GetCountry().c_str(),
			pGeo->GetIPAddress().GetHostAsString().c_str()
			);
	}

	ImGui::End();
}