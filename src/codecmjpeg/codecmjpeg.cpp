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
#include "streammjpeg.h"

#include <iostream>

#include <curl/curl.h>
#include <imgui/imgui.h>

#include <stdio.h>

IMPLEMENT_PLUGIN(CodecMJPEG)

CodecMJPEG::CodecMJPEG()
{
	
}

CodecMJPEG::~CodecMJPEG()
{

}

bool CodecMJPEG::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

// Messages:
// - stream_request: received when the user opens a camera.
// - stream_request_accepted: sent if the codec is capable of handling the requested camera.
// - stream_frame: sent when a frame has been written to the buffer.
// - stream_stopped: stop streaming the requested camera.
void CodecMJPEG::OnMessageReceived(const nlohmann::json& message)
{
	std::string messageType = message["type"];
	if (messageType == "stream_request")
	{
		const json& url = message["url"];
		const json& textureId = message["texture_id"];
		if (url.is_string() && textureId.is_number_unsigned())
		{
			ProcessStreamRequest(url.get<std::string>(), textureId.get<uint32_t>());
		}
	}
	else if (messageType == "stream_stopped")
	{
		const std::string& url = message["url"];
		m_Streams.remove_if([&url](const StreamMJPEGSharedPtr& pStream) { return pStream->GetUrl() == url; });
	}
	else if (messageType == "update")
	{
		for (StreamMJPEGSharedPtr pStream : m_Streams)
		{
			pStream->Update();
		}
	}
}

void CodecMJPEG::DrawUI(ImGuiContext* pContext)
{

}

void CodecMJPEG::ProcessStreamRequest(const std::string& url, uint32_t textureId)
{
	if (url.rfind(".mjpg") != std::string::npos)
	{
		std::string streamUrl = url;

		// If a "imagepath=" is present in the URL, the actual stream file is located
		// somewhere else.
		size_t imagePathPos = url.find("imagepath=");
		if (imagePathPos != std::string::npos)
		{
			size_t viewPos = url.find("/view/view.shtml");
			if (viewPos != std::string::npos)
			{
				// TODO: use the dynamic imagepath.
				streamUrl = url.substr(0, viewPos) + "/mjpg/video.mjpg";
			}
		}

		StreamMJPEGSharedPtr pStream = std::make_shared<StreamMJPEG>(streamUrl, textureId);
		m_Streams.push_back(pStream);
	}
}