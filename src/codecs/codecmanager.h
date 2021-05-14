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

#include <memory>
#include <string>
#include <vector>

#include "codecs/codec.h"

namespace Watcher
{

class CodecManager;
using CodecManagerUniquePtr = std::unique_ptr<CodecManager>;

class CodecManager
{
public:
    CodecManager();

    void StreamStart(const std::string& url, uint32_t textureId);

private:
    using CodecUniquePtr = std::unique_ptr<Codec>;
    using CodecVector = std::vector<CodecUniquePtr>;
    CodecVector m_Codecs;
};

} // namespace Watcher
