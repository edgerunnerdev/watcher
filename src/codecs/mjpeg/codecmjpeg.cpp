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

namespace Watcher
{

bool CodecMJPEG::CanDecode(const std::string& url) const
{
    return url.rfind(".mjpg") != std::string::npos;
}

StreamSharedPtr CodecMJPEG::CreateStream(const std::string& url, uint32_t textureId)
{
    if (!CanDecode(url))
    {
        return nullptr;
    }

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

    return std::make_shared<StreamMJPEG>(streamUrl, textureId);
}

} // namespace Watcher
