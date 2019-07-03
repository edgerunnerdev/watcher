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

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include "query.h"

//////////////////////////////////////////////////////////////////////////
// Query
//////////////////////////////////////////////////////////////////////////

Query::Query(const std::string& query):
m_Query(query)
{
	EncodeQuery();
}

void Query::EncodeQuery()
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::value_type c : m_Query) 
	{
		// Keep alphanumeric and other accepted characters intact.
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') 
		{
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded.
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	m_EncodedQuery = escaped.str();
}

const std::string& Query::Get() const
{
	return m_Query;
}

const std::string& Query::GetEncoded() const
{
	return m_EncodedQuery;
}


//////////////////////////////////////////////////////////////////////////
// QueryState
//////////////////////////////////////////////////////////////////////////

QueryState::QueryState()
{
	m_LastExecution = std::time(0);
	m_ResultCount = 0;
	m_CurrentStart = 0;
	m_Valid = false;
}

QueryState::QueryState(int start, int results)
{
	m_LastExecution = std::time(0);
	m_ResultCount = results;
	m_CurrentStart = start;
	m_Valid = true;
}

std::time_t QueryState::GetLastExecution() const
{
	return m_LastExecution;
}

void QueryState::SetLastExecution(std::time_t value)
{
	m_LastExecution = value;
}

int QueryState::GetResultCount() const
{
	return m_ResultCount;
}

int QueryState::GetCurrentStart() const
{
	return m_CurrentStart;
}

void QueryState::SetCurrentStart(int value)
{
	m_CurrentStart = value;
}

bool QueryState::IsCompleted() const
{
	return IsValid() && GetCurrentStart() > GetResultCount();
}

bool QueryState::IsValid() const
{
	return m_Valid;
}


//////////////////////////////////////////////////////////////////////////
// QueryResult
//////////////////////////////////////////////////////////////////////////

QueryResult::QueryResult(const std::string& url, const std::string& title)
{
	m_Url = url;
	m_Title = title;
}
