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

#include <string>

#include "geo_info.h"

class Camera
{
public:
	enum State
	{
		Unknown,
		Unauthorised,
		Active
	};

	Camera(const std::string& title, State cameraState);

	const std::string& GetTitle() const;
	GeoInfo::Handle GetGeoInfoHandle() const;
	void SetGeoInfoHandle(GeoInfo::Handle handle);
	State GetState() const;
	void SetState(State state);


private:
	std::string m_Title;
	GeoInfo::Handle m_GeoInfoHandle;
	State m_State;
};

inline Camera::Camera(const std::string& title, State cameraState)
{
	m_Title = title;
	m_GeoInfoHandle = GeoInfo::InvalidHandle;
	m_State = cameraState;
}

inline const std::string& Camera::GetTitle() const
{
	return m_Title;
}

inline GeoInfo::Handle Camera::GetGeoInfoHandle() const
{
	return m_GeoInfoHandle;
}

inline void Camera::SetGeoInfoHandle(GeoInfo::Handle handle)
{
	m_GeoInfoHandle = handle;
}

inline Camera::State Camera::GetState() const
{
	return m_State;
}

inline void Camera::SetState(State state)
{
	m_State = state;
}
