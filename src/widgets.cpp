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

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "widgets.h"

namespace ImGuiExt
{

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.txt)
void HelpMarker(const std::string& desc)
{
    using namespace ImGui;

    TextDisabled("(?)");
    if (IsItemHovered())
    {
        BeginTooltip();
        PushTextWrapPos(GetFontSize() * 35.0f);
        TextUnformatted(desc.c_str());
        PopTextWrapPos();
        EndTooltip();
    }
}

void Spinner(float radius, int thickness, const ImU32& color) 
{
    using namespace ImGui;

    ImDrawList* pDrawList = GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImGuiStyle& style = ImGui::GetStyle();
        
    pDrawList->PathClear();
    
    const float t = ImGui::GetTime();
    const int num_segments = 30;
    const int start = static_cast<int>(std::abs(sinf(t * 1.8f) * (num_segments - 5)));
    
    const float a_min = static_cast<float>(M_PI * 2.0f * static_cast<float>(start) / static_cast<float>(num_segments));
    const float a_max = static_cast<float>(M_PI * 2.0f * static_cast<float>(num_segments-3) / static_cast<float>(num_segments));

    const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);
    for (int i = 0; i < num_segments; i++) 
    {
        const float a = a_min + static_cast<float>(i) / static_cast<float>(num_segments) * (a_max - a_min);
        pDrawList->PathLineTo(ImVec2(centre.x + cosf(a + t * 8) * radius, centre.y + sinf(a + t * 8) * radius));
    }

    pDrawList->PathStroke(color, false, static_cast<float>(thickness));
}

} // namespace ImGuiExt
