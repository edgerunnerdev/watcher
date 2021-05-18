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

#include "imgui/imgui.h"
#include "imguiext/widgets.h"
#include "tasks/task.h"
#include "watcher.h"

namespace Watcher
{

Task::Task(const std::string& name) :
m_Name(name),
m_State(State::Disabled)
{

}

Task::~Task()
{

}

void Task::Update(float delta)
{

}

void Task::Start()
{
    
}

void Task::Stop()
{

}

void Task::Enable()
{
    m_State = State::Idle;
}

void Task::Disable()
{
    Stop();
    m_State = State::Disabled;
}

void Task::Render()
{
    using namespace ImGui;
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    BeginChild(m_Name.c_str(), ImVec2(0.0f, 40.0f), true);

    ImVec2 pos = GetCursorScreenPos();
    
    if (GetState() == State::Idle)
    {
        SetCursorScreenPos(ImVec2(pos.x + 8, pos.y + 4));
        Spinner("spinner", 10.0f, 4, IM_COL32(120, 120, 120, 255));
        SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 4));
        TextDisabled("%s", m_Name.c_str());
        SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 20));
        TextDisabled("Idle");
    }
	else if (GetState() == State::Disabled)
	{
		SetCursorScreenPos(ImVec2(pos.x + 8, pos.y + 4));
		Spinner("spinner", 10.0f, 4, IM_COL32(120, 120, 120, 255));
		SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 4));
		TextDisabled("%s", m_Name.c_str());
		SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 20));
		TextDisabled("Disabled");
	}
	else if (GetState() == State::Error)
	{
		SetCursorScreenPos(ImVec2(pos.x + 8, pos.y + 4));
		Spinner("spinner", 10.0f, 4, IM_COL32(120, 120, 120, 255));
		SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 4));
		TextDisabled("%s", m_Name.c_str());
		SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 20));
        if (m_Error.empty())
        {
            TextDisabled("Error");
        }
        else
        {
            TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", m_Error.c_str());
        }
	}
    else
    {
        SetCursorScreenPos(ImVec2(pos.x + 8, pos.y + 4));
        Spinner("spinner", 10.0f, 4, IM_COL32(0, 255, 255, 255));
        SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 4));
        Text("%s", m_Name.c_str());
        SetCursorScreenPos(ImVec2(pos.x + 38, pos.y + 20));
        Text("Running on localhost");
    }

    EndChild();
    PopStyleVar();
}

} // namespace Watcher
