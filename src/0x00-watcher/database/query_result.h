#pragma once

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Database
{

using QueryResultCell = std::optional< std::variant< int, double, std::string > >;
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