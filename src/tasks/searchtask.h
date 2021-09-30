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

#include <filesystem>
#include <vector>

#include "database/databasetime.h"
#include "database/queryresult.h"
#include "tasks/task.h"

namespace Watcher
{

///////////////////////////////////////////////////////////////////////////////
// SearchTask
// A task intended to be used for searching for cameras, using a search
// engine and linked to a database.
///////////////////////////////////////////////////////////////////////////////

class SearchTask : public Task
{
public:
	SearchTask(const std::string& name, const std::filesystem::path& defaultQueriesFilePath);
	virtual ~SearchTask() override;
	virtual void Update(float delta) override;

	virtual void Start() override;
	virtual void Stop() override;
	virtual void OnDatabaseCreated(Database* pDatabase) override;

	void AddQuery(const std::string& query);
	void RemoveQuery(const std::string& query);

	virtual void OnResult(const std::string& result, const DatabaseTime& time);

protected:
	const std::filesystem::path& GetDefaultQueriesFilePath() const;
	void AddQueriesFromDatabase();
	void AddQueryToDatabase(const std::string& query);
	void AddQueryInternal(const std::string& query);
	static void DatabaseSelectQueriesCallback(const QueryResult& result, void* pData);

	struct QueryData
	{
		std::string query;
		DatabaseTime time;
	};
	std::vector<QueryData> m_Queries;
	bool m_QueriesAddedFromDatabase;

private:
	std::filesystem::path m_DefaultQueriesFilePath;
};

inline const std::filesystem::path& SearchTask::GetDefaultQueriesFilePath() const
{
	return m_DefaultQueriesFilePath;
}

} // namespace Watcher
