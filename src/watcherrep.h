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

#include <array>
#include <memory>
#include <string>

#include <SDL_opengl.h>

#include "camera.h"
#include "camerarep.h"

struct SDL_Surface;
struct SDL_Window;

namespace Watcher
{

class Atlas;
using AtlasUniquePtr = std::unique_ptr<Atlas>;
class CommandBar;
using CommandBarUniquePtr = std::unique_ptr<CommandBar>;

class WatcherRep
{
public:
	WatcherRep(SDL_Window* pWindow);
	~WatcherRep();

	void ProcessEvent(const SDL_Event& event);
	void Update(float delta);
	void Render();

private:
	void SetUserInterfaceStyle();
	CameraVector GetHoveredCameras();

	void RenderCameras();
	void OpenPickedCamera();
	void FlushClosedCameras();
	const ImColor& GetPinColor(Camera::State state) const;

	SDL_Window* m_pWindow;
	AtlasUniquePtr m_pAtlas;
	float m_CellSize;
    CommandBarUniquePtr m_pCommandBar;
	
	using CameraRepList = std::list<CameraRep>;
	CameraRepList m_CameraReps;
	bool m_SelectCamera;

	std::array<ImColor, static_cast<size_t>(Camera::State::Count)> m_PinColor;
};

} // namespace Watcher
