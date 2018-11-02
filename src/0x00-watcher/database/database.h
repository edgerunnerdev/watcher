#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "prepared_statement.h"

struct sqlite3;

namespace Database
{

class Database
{
public:
	friend PreparedStatement;

	Database( const std::string& filename );
	~Database();
	void Update( float delta );
	void Execute( PreparedStatement statement );

private:
	static void ThreadMain( Database* pDatabase );
	void LaunchThread();
	void ConsumeStatements();
	void ExecuteActiveStatements();
	void BlockingQuery( const std::string& query );

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

}