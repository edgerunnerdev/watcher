#include <cassert>
#include "sqlite/sqlite3.h"
#include "database/database.h"
#include "database/query_result.h"
#include "log.h"
#include "prepared_statement.h"

namespace Database
{

PreparedStatement::PreparedStatement( Database* pDatabase, const std::string& query, QueryResultCallback pCallback /* = nullptr */, void* pCallbackData /* = nullptr */ ) :
m_Query( query ),
m_pStatement( nullptr ),
m_Executed( false ),
m_pCallback( pCallback ),
m_pCallbackData( pCallbackData )
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

void PreparedStatement::Bind( unsigned int index, double value )
{
	if ( sqlite3_bind_double( m_pStatement, index, value ) != SQLITE_OK )
	{
		Log::Error( "Error binding value to prepared statement." );
	}
}

void PreparedStatement::Execute()
{
	assert( !m_Executed );

	QueryResult result;
	int numColumns = sqlite3_column_count( m_pStatement );
	while ( 1 )
	{
		int rc = sqlite3_step( m_pStatement );
		if ( rc == SQLITE_ROW )
		{
			QueryResultRow row;
			for ( int i = 0; i < numColumns; i++ )
			{
				int columnType = sqlite3_column_type( m_pStatement, i );
				if ( columnType == SQLITE3_TEXT )
				{
					row.emplace_back( std::string( reinterpret_cast< const char* >( sqlite3_column_text( m_pStatement, i ) ) ) );
				}
				else if ( columnType == SQLITE_INTEGER )
				{
					row.emplace_back( sqlite3_column_int( m_pStatement, i ) );
				}
				else if ( columnType == SQLITE_FLOAT )
				{
					row.emplace_back( sqlite3_column_double( m_pStatement, i ) );
				}
			}
			result.Add( row );
		}
		else if ( rc == SQLITE_DONE )
		{
			break;
		}
		else
		{
			Log::Error( "Error during PreparedStatement::Execute" );
		}
	}

	if ( m_pCallback )
	{
		m_pCallback( result, m_pCallbackData );
	}

	sqlite3_finalize( m_pStatement );
	m_Executed = true;
}

}
