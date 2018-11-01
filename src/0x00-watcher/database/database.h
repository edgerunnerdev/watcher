#pragma once

#include <string>
#include "prepared_statement.h"

struct sqlite3;

namespace Database
{

class Database
{
public:
	friend PreparedStatement;

	Database( const std::string& filename );
	~Database();
	void Update( float delta );

private:
	float m_UpdateTimer;
	sqlite3* m_pDatabase;
};

}