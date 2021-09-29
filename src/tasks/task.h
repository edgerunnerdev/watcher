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

class Database;

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

	virtual void Start();
	virtual void Stop();
	virtual void OnDatabaseCreated(Database* pDatabase);

	void Enable();
	void Disable();
	void Render();

	const std::string& GetName() const;

protected:
	State GetState() const;
	void SetState(State state);
	void SetErrorString(const std::string& error);

private:
	std::string m_Name;
	State m_State;
	std::string m_Error;
	float m_SpinnerTimer;
};

inline const std::string& Task::GetName() const
{
	return m_Name;
}

inline Task::State Task::GetState() const
{
	return m_State;
}

inline void Task::SetState(State state)
{
	m_State = state;
}

inline void Task::SetErrorString(const std::string& error)
{
	m_Error = error;
}

} // namespace Watcher
