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

#include <list>
#include <memory>

#include "codecs/codec.h"

namespace Watcher
{

class CodecMJPEG : public Codec
{
public:
    CodecMJPEG() {}
    virtual bool CanDecode(const std::string& url) const override;
    virtual StreamSharedPtr CreateStream(const std::string& url, uint32_t textureId) override;
};

} // namespace Watcher