#include "sqlite/sqlite3.h"
#include "database.h"
#include "log.h"

namespace Database
{

Database::Database( const std::string& filename ) :
m_UpdateTimer( 0.0f ),
m_pDatabase( nullptr )
{
	if ( sqlite3_open( filename.c_str(), &m_pDatabase ) != SQLITE_OK )
	{
		Log::Error( "Couldn't open database '%s'", filename.c_str() );
	}
}

Database::~Database()
{
	if ( m_pDatabase != nullptr )
	{
		sqlite3_close( m_pDatabase );
	}
}

void Database::Update( float delta )
{
	static const float sDatabaseRefreshInterval = 1.0f;
	m_UpdateTimer -= delta;
	if ( m_UpdateTimer <= 0.0f )
	{
		m_UpdateTimer = sDatabaseRefreshInterval;
	}
}

}