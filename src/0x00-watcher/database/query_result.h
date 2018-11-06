#include <string>
#include <utility>
#include <vector>

namespace Database
{

using QueryResultPair = std::pair< std::string, std::string >;
using QueryResultPairVector = std::vector< QueryResultPair >;

class QueryResult
{
public:
	QueryResult( int argc, char** argv, char** azColName );
	const QueryResultPairVector& Get() const;

private:
	QueryResultPairVector m_Results;
};

using QueryResultCallback = void (*)( const QueryResult& result, void* pData );

}