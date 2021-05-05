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

#include <string>

namespace Watcher
{

///////////////////////////////////////////////////////////////////////////////
// Task
// A background task, performing work over time without requiring interaction
// by the user. These are used to discover new cameras in various ways, as well
// as where they're located, etc.
///////////////////////////////////////////////////////////////////////////////

class Task
{
public:
	enum class State
	{
		Disabled,
		Idle,
		Running,
		Error
	};

	Task(const std::string& name);
	virtual ~Task();
	virtual void Update(float delta);

	void Draw();

private:
	std::string m_Name;
	State m_State;
};

} // namespace Watcher
