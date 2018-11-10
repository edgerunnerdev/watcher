#pragma once

#ifdef __linux__
#include <experimental/optional>
#else
#include <optional>
#endif
#include <string>
#include <utility>
#include <vector>

namespace Database
{

// Less than ideal but at the moment std::variant isn't easily
// usable in GCC / clang without messing around.
class QueryResultType
{
public:
	QueryResultType( int value );
	QueryResultType( double value );
	QueryResultType( const std::string& value );

	int GetInt() const;
	double GetDouble() const;
	const std::string& GetString() const;

private:
	int m_Int;
	double m_Double;
	std::string m_String;
};

#ifdef __linux
using QueryResultCell = std::experimental::optional< QueryResultType >;
#else
using QueryResultCell = std::optional< QueryResultType >;
#endif
using QueryResultRow = std::vector< QueryResultCell >;
using QueryResultTable = std::vector< QueryResultRow >;

class QueryResult
{
public:
	void Add( const QueryResultRow& row );
	const QueryResultTable& Get() const;

private:
	QueryResultTable m_Result;
};

inline void QueryResult::Add( const QueryResultRow& row ) { m_Result.push_back( row ); }
inline const QueryResultTable& QueryResult::Get() const { return m_Result; }

using QueryResultCallback = void (*)( const QueryResult& result, void* pData );

}