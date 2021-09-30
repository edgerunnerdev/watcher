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

#include <ctime>
#include <string>
#include <vector>

namespace Watcher {
namespace Tasks {

//////////////////////////////////////////////////////////////////////////
// Query
//////////////////////////////////////////////////////////////////////////

class Query
{
public:
	Query() {}
	Query(const std::string& query);

	const std::string& Get() const;
	const std::string& GetEncoded() const;

private:
	void EncodeQuery();
	std::string m_Query;
	std::string m_EncodedQuery;
};


//////////////////////////////////////////////////////////////////////////
// QueryState
//
// As Google's API only returns 10 results at a time, we need to keep track of
// how many results we have processed and keep querying for more results.
// We also keep track of when the query has last been executed, so we aren't
// repeating the same queries every time (this also helps with the API's rate
// limiting of 1000 queries/day).
//////////////////////////////////////////////////////////////////////////

class QueryState
{
public:
	QueryState();

	std::time_t GetLastExecution() const;
	void SetLastExecution(std::time_t value);
	int GetResultCount() const;
	void SetResultCount(int value);
	int GetCurrentStart() const;
	void SetCurrentStart(int value);
	bool IsCompleted() const;
	bool IsValid() const;

private:
	std::time_t m_LastExecution;
	int m_ResultCount;
	int m_CurrentStart;
};


//////////////////////////////////////////////////////////////////////////
// QueryResult
//////////////////////////////////////////////////////////////////////////

class QueryResult
{
public:
	QueryResult(const std::string& url, const std::string& title);

	const std::string& GetUrl() const;
	const std::string& GetTitle() const;

private:
	std::string m_Url;
	std::string m_Title;
};

using QueryResults = std::vector<QueryResult>;

} // namespace Tasks
} // namespace Watcher
