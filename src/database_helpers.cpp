#include <SDL.h>
#include "sqlite/sqlite3.h"
#include "database_helpers.h"

void ExecuteDatabaseQuery( sqlite3* pDatabase, const std::string& query )
{
	char* pError = nullptr;
	int rc = sqlite3_exec( pDatabase, query.c_str(), nullptr, 0, &pError );
	if( rc != SQLITE_OK )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "SQL query error", pError, nullptr );
		sqlite3_free( pError );
	}
}
