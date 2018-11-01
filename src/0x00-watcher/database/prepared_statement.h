#pragma once

#include <string>

struct sqlite3_stmt;

namespace Database
{

class Database;

class PreparedStatement
{
public:
	PreparedStatement( Database* pDatabase, const std::string& query ); 

	void Bind( unsigned int index, const std::string& text );
	void Bind( unsigned int index, int value );

private:
	std::string m_Query;
	sqlite3_stmt* m_pStatement;
};

}