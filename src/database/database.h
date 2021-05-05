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

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "prepared_statement.h"

struct sqlite3;

namespace Watcher
{

class Database;
using DatabaseUniquePtr = std::unique_ptr< Database >;

class Database
{
public:
	friend PreparedStatement;

	Database( const std::string& filename );
	~Database();
	void Execute( PreparedStatement statement );

private:
	static void sThreadMain( Database* pDatabase );
	void ConsumeStatements();
	void ExecuteActiveStatements();
	void BlockingNonQuery( const std::string& query );

	using StatementVector = std::vector< PreparedStatement >;
	sqlite3* m_pDatabase;

	// Whenever Execute is called, the statement is added
	// to the "pending" list.
	std::mutex m_PendingStatementsMutex;
	StatementVector m_PendingStatements;

	// The "pending" list is regularly shunted into the "active"
	// list, which is then processed by a thread in bulk and the
	// underlying SQLite operations are actually performed.
	std::mutex m_ActiveStatementsMutex;
	StatementVector m_ActiveStatements;

	std::atomic_bool m_RunThread;
	std::thread m_Thread;
};

} // namespace Watcher
