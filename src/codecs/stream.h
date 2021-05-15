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

namespace Watcher
{

class Stream;
using StreamSharedPtr = std::shared_ptr<Stream>;

class Stream
{
public:
    Stream(const std::string& url, uint32_t textureId);
    virtual ~Stream() {}
    virtual void Update() = 0;
    virtual const std::string& GetError() const = 0;

    enum class State
    {
        Initialising,
        Streaming,
        Error,
        Terminated
    };
    State GetState() const;

protected:
    const std::string& GetUrl() const;
    uint32_t GetTextureId() const;

private:
    std::string m_Url;
    uint32_t m_TextureId;
    State m_State;
};

inline Stream::Stream(const std::string& url, uint32_t textureId) :
m_Url(url),
m_TextureId(textureId),
m_State(State::Initialising)
{
}

inline Stream::State Stream::GetState() const
{
    return m_State;
}

inline const std::string& Stream::GetUrl() const
{
    return m_Url;
}

inline uint32_t Stream::GetTextureId() const
{
    return m_TextureId;
}

} // namespace Watcher
