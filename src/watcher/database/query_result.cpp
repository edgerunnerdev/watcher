#include "database/query_result.h"

namespace Database
{

QueryResultType::QueryResultType( int value ) :
m_Int( value ),
m_Double( 0.0 )
{

}

QueryResultType::QueryResultType( double value ) :
m_Int( 0 ),
m_Double( value )
{

}

QueryResultType::QueryResultType( const std::string& value ) :
m_Int( 0 ),
m_Double( 0.0 )
{
	m_String = value;
}

int QueryResultType::GetInt() const
{
	return m_Int;
}

double QueryResultType::GetDouble() const
{
	return m_Double;
}

const std::string& QueryResultType::GetString() const
{
	return m_String;
}

}