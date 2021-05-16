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

#include <cassert>
#include "sqlite/sqlite3.h"
#include "database/database.h"
#include "database/queryresult.h"
#include "log.h"
#include "preparedstatement.h"

namespace Watcher
{

PreparedStatement::PreparedStatement(Database* pDatabase, const std::string& query, QueryResultCallback pCallback /* = nullptr */, void* pCallbackData /* = nullptr */) :
	m_Query(query),
	m_pStatement(nullptr),
	m_Executed(false),
	m_pCallback(pCallback),
	m_pCallbackData(pCallbackData)
{
	if (sqlite3_prepare_v2(pDatabase->m_pDatabase, query.c_str(), -1, &m_pStatement, nullptr) != SQLITE_OK)
	{
		Log::Error("sqlite3_prepare_v2 failed for query '%s' with error '%d': %s.",
			query.c_str(),
			sqlite3_errcode(pDatabase->m_pDatabase),
			sqlite3_errmsg(pDatabase->m_pDatabase)
		);
	}
}

void PreparedStatement::Bind(unsigned int index, const std::string& text)
{
	if (sqlite3_bind_text(m_pStatement, index, text.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
	{
		Log::Error("Error binding value to prepared statement.");
	}
}

void PreparedStatement::Bind(unsigned int index, int value)
{
	if (sqlite3_bind_int(m_pStatement, index, value) != SQLITE_OK)
	{
		Log::Error("Error binding value to prepared statement.");
	}
}

void PreparedStatement::Bind(unsigned int index, double value)
{
	if (sqlite3_bind_double(m_pStatement, index, value) != SQLITE_OK)
	{
		Log::Error("Error binding value to prepared statement.");
	}
}

void PreparedStatement::ExecuteBlocking()
{
	assert(!m_Executed);

	QueryResult result;
	int numColumns = sqlite3_column_count(m_pStatement);
	while (1)
	{
		int rc = sqlite3_step(m_pStatement);
		if (rc == SQLITE_ROW)
		{
			QueryResultRow row;
			for (int i = 0; i < numColumns; i++)
			{
				int columnType = sqlite3_column_type(m_pStatement, i);
				if (columnType == SQLITE3_TEXT)
				{
					row.emplace_back(std::string(reinterpret_cast<const char*>(sqlite3_column_text(m_pStatement, i))));
				}
				else if (columnType == SQLITE_INTEGER)
				{
					row.emplace_back(sqlite3_column_int(m_pStatement, i));
				}
				else if (columnType == SQLITE_FLOAT)
				{
					row.emplace_back(sqlite3_column_double(m_pStatement, i));
				}
			}
			result.Add(row);
		}
		else if (rc == SQLITE_DONE)
		{
			break;
		}
		else
		{
			Log::Error("Error during PreparedStatement::Execute: %s", sqlite3_errstr(rc));
			break;
		}
	}

	if (m_pCallback)
	{
		m_pCallback(result, m_pCallbackData);
	}

	sqlite3_finalize(m_pStatement);
	m_Executed = true;
}

} // namespace Watcher
