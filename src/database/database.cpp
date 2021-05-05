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

#include <chrono>
#include "sqlite/sqlite3.h"
#include "database.h"
#include "log.h"

namespace Watcher
{

Database::Database( const std::string& filename ) :
m_pDatabase( nullptr ),
m_RunThread( true )
{
	if ( sqlite3_open( filename.c_str(), &m_pDatabase ) != SQLITE_OK )
	{
		Log::Error( "Couldn't open database '%s'", filename.c_str() );
	}

	m_Thread = std::thread( sThreadMain, this );
}

Database::~Database()
{
	m_RunThread = false;
	if ( m_Thread.joinable() )
	{
		m_Thread.join();
	}

	if ( m_pDatabase != nullptr )
	{
		sqlite3_close( m_pDatabase );
	}
}

void Database::Execute( PreparedStatement statement )
{
	std::lock_guard< std::mutex > pendingLock( m_PendingStatementsMutex );
	m_PendingStatements.push_back( statement );
}

void Database::sThreadMain( Database* pDatabase )
{
	while ( pDatabase->m_RunThread )
	{
		pDatabase->ConsumeStatements();
		pDatabase->ExecuteActiveStatements();
		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
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
	std::lock_guard< std::mutex > activeLock( m_ActiveStatementsMutex );
	if ( m_ActiveStatements.empty() == false )
	{
		BlockingNonQuery( "BEGIN TRANSACTION;" );
		for ( PreparedStatement& statement : m_ActiveStatements )
		{
			statement.Execute();
		}
		m_ActiveStatements.clear();
		BlockingNonQuery( "COMMIT;" );
	}
}

void Database::BlockingNonQuery( const std::string& query )
{
	char* pError = nullptr;
	while ( 1 )
	{
		int rc = sqlite3_exec( m_pDatabase, query.c_str(), nullptr, 0, &pError );
		if ( rc != SQLITE_OK && rc != SQLITE_BUSY )
		{
			Log::Error( "SQL query error: %s", pError );
			sqlite3_free( pError );		
		}
		else if ( rc == SQLITE_OK )
		{
			break;
		}
	}
}

} // namespace Watcher
