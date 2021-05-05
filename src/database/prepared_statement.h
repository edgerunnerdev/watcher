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

#include "database/query_result.h"

struct sqlite3_stmt;

namespace Watcher
{

class Database;

class PreparedStatement
{
public:
	PreparedStatement( Database* pDatabase, const std::string& query, QueryResultCallback pCallback = nullptr, void* pCallbackData = nullptr ); 
	void Execute();
	void Bind( unsigned int index, const std::string& text );
	void Bind( unsigned int index, int value );
	void Bind( unsigned int index, double value );

private:
	std::string m_Query;
	sqlite3_stmt* m_pStatement;
	bool m_Executed;
	QueryResultCallback m_pCallback;
	void* m_pCallbackData;
};

} // namespace Watcher
