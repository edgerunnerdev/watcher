// codecmjpeg.cpp : Defines the exported functions for the DLL application.
//

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

#include "codecmjpeg.h"

#include <iostream>

#include <curl/curl.h>
#include <imgui/imgui.h>

#include <stdio.h>

IMPLEMENT_PLUGIN(CodecMJPEG)

// This needs to be a static function as libcurl is a C library and will segfault if passed
// a local lambda.
static size_t WriteMemoryCallback(void* pContents, size_t size, size_t nmemb, void* pUserData)
{
	size_t realSize = size * nmemb;
	std::string& data = *reinterpret_cast<std::string*>(pUserData);
	size_t curDataSize = data.size();
	data.resize(curDataSize + realSize);
	memcpy(&data[curDataSize], pContents, realSize);
	return realSize;
}

CodecMJPEG::CodecMJPEG()
{
	m_pCurlHandle = curl_easy_init();
}

CodecMJPEG::~CodecMJPEG()
{
	curl_easy_cleanup(m_pCurlHandle);
}

bool CodecMJPEG::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

// Messages:
// - stream_request: received when the user opens a camera.
// - stream_request_accepted: sent if the codec is capable of handling the requested camera.
// - stream_frame_info: received when watcher allocates a buffer for the codec to write a frame to.
// - stream_frame: sent when a frame has been written to the buffer.
void CodecMJPEG::OnMessageReceived(const nlohmann::json& message)
{
	std::string messageType = message["type"];
	if (messageType == "stream_request")
	{
		const json& url = message["url"];
		if (url.is_string())
		{
			ProcessStreamRequest(url.get<std::string>());
		}
	}
	else if (messageType == "stream_frame_info")
	{

	}
}

void CodecMJPEG::DrawUI(ImGuiContext* pContext)
{
	//ImGui::SetCurrentContext(pContext);

	//if (ImGui::CollapsingHeader("Geolocation", ImGuiTreeNodeFlags_DefaultOpen))
	//{
	//	std::lock_guard< std::mutex > lock(m_AccessMutex);
	//	ImGui::Text("Provider: ipinfo.io");

	//	if (m_RateLimitExceeded)
	//	{
	//		ImGui::Text("Rate limit exceeded.");
	//	}
	//	else
	//	{
	//		std::stringstream ss;
	//		ss << "Queue size: " << m_Queue.size();
	//		ImGui::Text(ss.str().c_str());
	//	}
	//}
}

void CodecMJPEG::ProcessStreamRequest(const std::string& url)
{

	//json message =
	//{
	//	{ "type", "stream_request_accepted" },
	//	{ "url", url },
	//	{ "codec", "CodecMJPEG" }
	//};
	//pDetector->m_pMessageCallback(message);
}