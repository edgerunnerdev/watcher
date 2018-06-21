#pragma once

#include <string>

struct sqlite3;
struct sqlite3_stmt;

void ExecuteDatabaseQuery( sqlite3* pDatabase, const std::string& query );
void ExecuteDatabaseQuery( sqlite3* pDatabase, sqlite3_stmt* pStatement );