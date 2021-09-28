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

#include "codecs/codecmanager.h"
#include "imgui/imgui.h"
#include "camerarep.h"
#include "watcher.h"

namespace Watcher
{

CameraRep::CameraRep(CameraWeakPtr cameraWeakPtr)
{
	m_CameraWeakPtr = cameraWeakPtr;
	m_WindowWidth = 480.0f;
	m_WindowHeight = 320.0f;
	m_TextureWidth = 0;
	m_TextureHeight = 0;
	glGenTextures(1, &m_Texture);
	m_Open = true;

    CameraSharedPtr pCamera = cameraWeakPtr.lock();
    m_pStream = g_pWatcher->GetCodecManager()->CreateStream(pCamera->GetURL(), m_Texture);
}

CameraWeakPtr CameraRep::GetCameraWeakPtr() const
{
	return m_CameraWeakPtr;
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

void CameraRep::ConstrainedRatio(ImGuiSizeCallbackData* pData) 
{ 
	CameraRep* pCameraRep = reinterpret_cast<CameraRep*>(pData->UserData);
	const float ratio = static_cast<float>(pCameraRep->m_TextureHeight) / static_cast<float>(pCameraRep->m_TextureWidth);
	pData->DesiredSize.y = pData->DesiredSize.x * ratio;
	pCameraRep->m_WindowWidth = pData->DesiredSize.x;
	pCameraRep->m_WindowHeight = pData->DesiredSize.y;
}

void CameraRep::Update()
{
    if (m_pStream != nullptr)
    {
        m_pStream->Update();
    }
}

void CameraRep::Render()
{
	if (!m_Open)
	{
		return;
	}

	CameraSharedPtr pCamera = m_CameraWeakPtr.lock();
	if (pCamera == nullptr)
	{
		return;
	}

	if (m_TextureWidth == 0 && m_TextureHeight == 0)
	{
		ImGui::SetNextWindowSize(ImVec2(m_WindowWidth, m_WindowHeight));
	}
	else
	{
		ImGui::SetNextWindowSizeConstraints(
			ImVec2(0, 0), 
			ImVec2((float)m_TextureWidth, (float)m_TextureHeight), 
			ConstrainedRatio,
			this
		);
	}

	if (ImGui::Begin(pCamera->GetURL().c_str(), &m_Open, ImGuiWindowFlags_NoSavedSettings))
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

				json message =
				{
					{ "type", "stream_started" },
					{ "url", pCamera->GetURL() }
				};
				g_pWatcher->OnMessageReceived(message);
			}
		}

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImTextureID cameraTexture = reinterpret_cast<ImTextureID>(GetTexture());
		pDrawList->AddImage(cameraTexture, windowPos, ImVec2(windowPos.x + m_WindowWidth, windowPos.y + m_WindowHeight));

		GeolocationData* pGeo = pCamera->GetGeolocationData();
		ImGui::Text("%s, %s, %s", pGeo->GetCity().c_str(), pGeo->GetRegion().c_str(), pGeo->GetCountry().c_str());
		ImGui::Text("%s", pGeo->GetIPAddress().ToString().c_str());

		if (m_Open == false)
		{
			Close();
		}
	}

	ImGui::End();
}

} // namespace Watcher
