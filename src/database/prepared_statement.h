#pragma once

#include <string>

#include "database/query_result.h"

struct sqlite3_stmt;

namespace Database
{

class Database;

class PreparedStatement
{
public:
	PreparedStatement( Database* pDatabase, const std::string& query, QueryResultCallback pCallback = nullptr, void* pCallbackData = nullptr ); 
	void Execute();
	void Bind( unsigned int index, const std::string& text );
	void Bind( unsigned int index, int value );
	void Bind( unsigned int index, double value );

private:
	std::string m_Query;
	sqlite3_stmt* m_pStatement;
	bool m_Executed;
	QueryResultCallback m_pCallback;
	void* m_pCallbackData;
};

}