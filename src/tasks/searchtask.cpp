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

#include <filesystem>
#include <fstream>
#include <string>

#include "ext/json.h"
#include "database/database.h"
#include "tasks/searchtask.h"
#include "log.h"

namespace Watcher
{

SearchTask::SearchTask(const std::string& name, const std::filesystem::path& defaultQueriesFilePath) : 
Task(name),
m_DefaultQueriesFilePath(defaultQueriesFilePath)
{

}

SearchTask::~SearchTask() 
{

}

void SearchTask::Update(float delta)
{
    Task::Update(delta);
}

void SearchTask::Start()
{
    Task::Start();
}

void SearchTask::Stop()
{
    Task::Stop();
}

void SearchTask::OnDatabaseCreated(Database* pDatabase)
{
    const std::filesystem::path& path = GetDefaultQueriesFilePath();
    if (std::filesystem::exists(path))
    {
        using json = nlohmann::json;
		std::ifstream file(path);
		if ( file.is_open() )
		{
			json data;
			file >> data;
			file.close();

			if (data.is_null())
			{
				return;
			}

			if (data.is_array() == false)
			{
				Log::Error("Invalid format for '%s': root must be an array.", path.c_str());
				return;
			}

			for (json& entry : data)
			{
				if (entry.is_string())
				{
					PreparedStatement statement(pDatabase, "INSERT INTO SearchQueries VALUES(?1, ?2, ?3);");
					statement.Bind(1, GetName());
					statement.Bind(2, entry.get<std::string>());
					statement.Bind(3, "");
					pDatabase->Execute(statement);
				}
			}
		}
    }
}

void SearchTask::AddQuery(const std::string& query)
{

}

void SearchTask::RemoveQuery(const std::string& query)
{

}

void SearchTask::OnQuery(const std::string& query)
{

}

void SearchTask::OnResult(const std::string& result, const DatabaseTime& time)
{

}

} // namespace Watcher
