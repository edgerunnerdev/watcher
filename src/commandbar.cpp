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

#include <imgui/imgui.h>

#include "atlas/tilestreamer.h"
#include "tasks/googlesearch/googlesearch.h"
#include "tasks/task.h"
#include "commandbar.h"
#include "watcher.h"

namespace Watcher
{

CommandBar::CommandBar()
{
    m_AnimTimer = 0.0f;
    m_ShowAtlasTileStreamer = false;
    m_ShowDemoWindow = false;
    m_ShowGoogleQueries = false;
}

void CommandBar::Render()
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
	ImGui::SetNextWindowSize(ImVec2(250, 0));
	ImGui::Begin("CommandBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("Quit");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Google queries", nullptr, m_ShowGoogleQueries))
            {
                m_ShowGoogleQueries = !m_ShowGoogleQueries;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Development"))
            {
                if (ImGui::MenuItem("Atlas tile streamer", nullptr, m_ShowAtlasTileStreamer))
                {
                    m_ShowAtlasTileStreamer = !m_ShowAtlasTileStreamer;
                }
                if (ImGui::MenuItem("ImGui demo window", nullptr, m_ShowDemoWindow))
                {
                    m_ShowDemoWindow = !m_ShowDemoWindow;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("About");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (m_ShowAtlasTileStreamer)
    {
        TileStreamer::ShowDebugUI(&m_ShowAtlasTileStreamer);
    }

    if (m_ShowDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_ShowDemoWindow);
    }

    if (m_ShowGoogleQueries)
    {
        Tasks::GoogleSearch* pTask = reinterpret_cast<Tasks::GoogleSearch*>(g_pWatcher->GetTask("Google search"));
        if (pTask != nullptr)
        {
            pTask->ShowQueriesUI(&m_ShowGoogleQueries);
        }
    }

    RenderSearchWidget();
    RenderTasks();
    ImGui::End();
}

void CommandBar::RenderSearchWidget()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
    ImGui::BeginChild("SearchWidget", ImVec2(0.0f, 30.0f), true);
    RenderSearchBackground();
    RenderSearchButton();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void CommandBar::RenderSearchBackground()
{
    const bool isSearching = g_pWatcher->IsSearching();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    const float width = ImGui::GetWindowWidth();
    const float height = ImGui::GetWindowHeight();

    ImVec2 tl(p);
    ImVec2 br(p.x + width, p.y + height);
    ImU32 bg = isSearching ? IM_COL32(0, 128, 128, 255) : IM_COL32(40, 40, 40, 255);
    ImU32 fg = isSearching ? IM_COL32(0, 255, 255, 255) : IM_COL32(80, 80, 80, 255);
    pDrawList->AddRectFilled(tl, br, bg);

    const float stripeWidth = 15.0f;
    const float stripeSlant = 15.0f;
    for (int i = 0; i < 14; ++i)
    {
        float xOffset = 30.0f * (i - 1) + m_AnimTimer;
        pDrawList->AddQuadFilled(
            ImVec2(tl.x + xOffset, tl.y),
            ImVec2(tl.x + xOffset + stripeWidth, tl.y),
            ImVec2(tl.x + xOffset + stripeWidth + stripeSlant, br.y),
            ImVec2(tl.x + xOffset + stripeSlant, br.y),
            fg
        );
    }

    if (g_pWatcher->IsSearching())
    {
        m_AnimTimer += ImGui::GetIO().DeltaTime * 5.0f;
        if (m_AnimTimer > stripeWidth * 2.0f)
        {
            m_AnimTimer = 0.0f;
        }
    }
}

void CommandBar::RenderSearchButton()
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    if (ImGui::InvisibleButton("SearchWidgetButton", ImGui::GetWindowSize()))
    {
        g_pWatcher->SetSearching(!g_pWatcher->IsSearching());
    }

    const float width = ImGui::GetWindowWidth();
    const float height = ImGui::GetWindowHeight();
    const ImVec2 padding(20, 6);
    const ImVec2 tl(p.x + padding.x, p.y + padding.y);
    const ImVec2 br(p.x + width - padding.x, p.y + height - padding.y);

    ImU32 borderColor = ImGui::IsItemHovered() ? IM_COL32(200, 200, 200, 255) : IM_COL32(128, 128, 128, 255);
    pDrawList->AddRectFilled(tl, br, IM_COL32(20, 20, 20, 200));
    pDrawList->AddRect(tl, br, borderColor);

    const std::string& text = g_pWatcher->IsSearching() ? "Searching" : "Begin search";
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    pDrawList->AddText(ImVec2(p.x + (width - textSize.x) / 2.0f, p.y + (height - ImGui::GetTextLineHeight()) / 2.0f), IM_COL32(255, 255, 255, 255), text.c_str());
}

void CommandBar::RenderTasks()
{
    const TaskVector& tasks = g_pWatcher->GetTasks();
    for (auto&& pTask : tasks)
    {
        pTask->Render();
    }
}

} // namespace Watcher
