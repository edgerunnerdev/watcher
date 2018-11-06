#include "database/query_result.h"

namespace Database
{

QueryResult::QueryResult( int argc, char** argv, char** azColName )
{
	for ( int i = 0; i < argc; i++ )
	{
		m_Results.emplace_back( azColName[ i ], argv[ i ] );
	}
}

const QueryResultPairVector& QueryResult::Get() const
{
	return m_Results;
}

}