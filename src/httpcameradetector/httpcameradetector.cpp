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

#include "httpcameradetector.h"

#include <iostream>

#include <curl/curl.h>
#include <imgui/imgui.h>

#include <stdio.h>

IMPLEMENT_PLUGIN(HTTPCameraDetector)

HTTPCameraDetector::HTTPCameraDetector()
{

}

HTTPCameraDetector::~HTTPCameraDetector()
{

}

bool HTTPCameraDetector::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

void HTTPCameraDetector::OnMessageReceived(const nlohmann::json& message)
{

}

void HTTPCameraDetector::DrawUI(ImGuiContext* pContext)
{
	ImGui::SetCurrentContext(pContext);

	if (ImGui::CollapsingHeader("HTTP camera detector", ImGuiTreeNodeFlags_DefaultOpen))
	{

	}
}
