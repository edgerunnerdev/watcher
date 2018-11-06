#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Database
{

using ColumnValue = std::optional< std::variant< int, double, std::string > >;
using ColumnVector = std::vector< ColumnValue >;

class QueryResult
{
public:
	void Add( int value );
	void Add( double value );
	void Add( const std::string& value );
	void Add( std::nullopt_t value );
	const ColumnVector& Get() const;

private:
	ColumnVector m_Results;
};

inline void QueryResult::Add( int value ) { m_Results.push_back( value ); }
inline void QueryResult::Add( double value ) { m_Results.push_back( value ); }
inline void QueryResult::Add( const std::string& value ) { m_Results.push_back( value ); }
inline void QueryResult::Add( std::nullopt_t ) { m_Results.push_back( std::nullopt ); }
inline const ColumnVector& QueryResult::Get() const { return m_Results; }

using QueryResultCallback = void (*)( const QueryResult& result, void* pData );

}