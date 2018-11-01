#include <cassert>
#include "sqlite/sqlite3.h"
#include "database.h"
#include "log.h"
#include "prepared_statement.h"

namespace Database
{

PreparedStatement::PreparedStatement( Database* pDatabase, const std::string& query ) :
m_Query( query )
{
	sqlite3_prepare_v2( pDatabase->m_pDatabase, query.c_str(), -1, &m_pStatement, nullptr );
}

void PreparedStatement::Bind( unsigned int index, const std::string& text )
{
	if ( sqlite3_bind_text( m_pStatement, index, text.c_str(), -1, SQLITE_TRANSIENT ) != SQLITE_OK )
	{
		Log::Error( "Error binding value to prepared statement." );
	}

}

void PreparedStatement::Bind( unsigned int index, int value )
{
	if ( sqlite3_bind_int( m_pStatement, index, value ) != SQLITE_OK )
	{
		Log::Error( "Error binding value to prepared statement." );
	}
}

}
