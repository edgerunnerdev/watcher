#include <SDL.h>
#include "sqlite/sqlite3.h"
#include "database_helpers.h"

void ExecuteDatabaseQuery( sqlite3* pDatabase, const std::string& query )
{
	while ( 1 )
	{
		char* pError = nullptr;
		int rc = sqlite3_exec( pDatabase, query.c_str(), nullptr, 0, &pError );
		if ( rc == SQLITE_OK )
		{
			break;
		}
		else
		{
			if ( rc != SQLITE_BUSY )
			{
				SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "SQL query error", pError, nullptr );
			}
			sqlite3_free( pError );
		}
	}
}

void ExecuteDatabaseQuery( sqlite3* pDatabase, sqlite3_stmt* pStatement )
{
	while ( 1 )
	{
		char* pError = nullptr;
		int rc = sqlite3_step( pStatement );
		if ( rc == SQLITE_DONE ) // Yes, it's SQLITE_DONE rather than SQLITE_OK. No idea why.
		{
			break;
		}
		else
		{
			if ( rc != SQLITE_BUSY )
			{
				SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "SQL query error", sqlite3_errmsg( pDatabase ), nullptr );
			}
		}
	}
}