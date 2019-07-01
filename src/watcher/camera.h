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
#include <vector>

#include <network/network.h>
#include "geolocationdata.h"

class Camera;
using CameraVector = std::vector<Camera>;

class Camera
{
public:
	enum State
	{
		Unknown,
		Unauthorised,
		Active
	};

	Camera(const std::string& title, const std::string& url, const Network::IPAddress& address, State cameraState);

	const std::string& GetTitle() const;
	const std::string& GetURL() const;
	const Network::IPAddress& GetAddress() const;
	GeolocationData* GetGeolocationData() const;
	void SetGeolocationData(GeolocationDataSharedPtr pGeolocationData);
	State GetState() const;
	void SetState(State state);

private:
	std::string m_Title;
	std::string m_URL;
	Network::IPAddress m_Address;
	GeolocationDataSharedPtr m_pGeolocationData;
	State m_State;
};

inline Camera::Camera(const std::string& title, const std::string& url, const Network::IPAddress& address, State cameraState)
{
	m_Title = title;
	m_URL = url;
	m_Address = address;
	m_State = cameraState;
}

inline const std::string& Camera::GetTitle() const
{
	return m_Title;
}

inline const std::string& Camera::GetURL() const
{
	return m_URL;
}

inline const Network::IPAddress& Camera::GetAddress() const
{
	return m_Address;
}

inline GeolocationData* Camera::GetGeolocationData() const
{
	return m_pGeolocationData.get();
}

inline void Camera::SetGeolocationData(GeolocationDataSharedPtr pGeolocationData)
{
	m_pGeolocationData = pGeolocationData;
}

inline Camera::State Camera::GetState() const
{
	return m_State;
}

inline void Camera::SetState(State state)
{
	m_State = state;
}
