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

#include "codecs/mjpeg/codecmjpeg.h"
#include "codecs/codecmanager.h"

namespace Watcher
{

CodecManager::CodecManager()
{
    m_Codecs.push_back(std::move(std::make_unique<CodecMJPEG>()));
}

StreamSharedPtr CodecManager::CreateStream(const std::string& url, uint32_t textureId)
{
    for (auto&& pCodec : m_Codecs)
    {
        if (pCodec->CanDecode(url))
        {
            return pCodec->CreateStream(url, textureId);
        }
    }

    return nullptr;
}

} // namespace Watcher