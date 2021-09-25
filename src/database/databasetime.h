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

#include <chrono>
#include <optional>
#include <string>

namespace Watcher
{

///////////////////////////////////////////////////////////////////////////////
// DatabaseTime
// Type used for getting date/time in and out of the database, as the DB stores
// it as plain text.
///////////////////////////////////////////////////////////////////////////////

class DatabaseTime
{
public:
	DatabaseTime();
	DatabaseTime(const std::string& text);

	using Type = std::optional<std::chrono::time_point<std::chrono::system_clock>>;
	Type Get() const;
	const std::string& GetAsString() const;

private:
	Type m_Type;
	std::string m_String;
};

inline DatabaseTime::Type DatabaseTime::Get() const
{
	return m_Type;
}

inline const std::string& DatabaseTime::GetAsString() const
{
	return m_String;
}

} // namespace Watcher
