///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

#include <imgui/imgui.h>

namespace Watcher
{

class CommandBar;
using CommandBarUniquePtr = std::unique_ptr<CommandBar>;

class CommandBar
{
public:
    CommandBar();

    void Render();

private:
    void RenderSearchWidget();
    void RenderSearchBackground();
    void RenderSearchButton();
    void RenderTasks();
    float m_AnimTimer;
    bool m_ShowAtlasTileStreamer;
    bool m_ShowDemoWindow;
    bool m_ShowGoogleQueries;
};

} // namespace Watcher
