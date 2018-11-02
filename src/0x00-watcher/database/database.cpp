#include "sqlite/sqlite3.h"
#include "database.h"
#include "log.h"

namespace Database
{

Database::Database( const std::string& filename ) :
m_pDatabase( nullptr ),
m_RunThread( true )
{
	if ( sqlite3_open( filename.c_str(), &m_pDatabase ) != SQLITE_OK )
	{
		Log::Error( "Couldn't open database '%s'", filename.c_str() );
	}

	LaunchThread();
}

Database::~Database()
{
	m_RunThread = false;
	m_Thread.join();

	if ( m_pDatabase != nullptr )
	{
		sqlite3_close( m_pDatabase );
	}
}

void Database::LaunchThread()
{

}

void Database::ThreadMain( Database* pDatabase )
{
	while ( pDatabase->m_RunThread )
	{
		pDatabase->ConsumeStatements();
		pDatabase->ExecuteActiveStatements();
	}
}

void Database::ConsumeStatements()
{
	std::lock_guard< std::mutex > pendingLock( m_PendingStatementsMutex );
	std::lock_guard< std::mutex > activeLock( m_ActiveStatementsMutex );
	for ( const PreparedStatement& statement : m_PendingStatements )
	{
		m_ActiveStatements.push_back( statement );
	}
	m_PendingStatements.clear();
}

void Database::ExecuteActiveStatements()
{
	BlockingQuery( "BEGIN TRANSACTION;" );
	std::lock_guard< std::mutex > activeLock( m_ActiveStatementsMutex );
	for ( PreparedStatement& statement : m_ActiveStatements )
	{
		statement.Execute();
	}
	m_ActiveStatements.clear();
	BlockingQuery( "COMMIT;" );
}

void Database::BlockingQuery( const std::string& query )
{
	char* pError = nullptr;
	int rc = sqlite3_exec( m_pDatabase, query.c_str(), nullptr, 0, &pError );
	if ( rc == SQLITE_OK )
	{
		
	}
	else
	{
		Log::Error( "SQL query error: %s", pError );
		sqlite3_free( pError );
	}
}

}