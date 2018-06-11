#pragma once

#include <string>

struct sqlite3;

void ExecuteDatabaseQuery( sqlite3* pDatabase, const std::string& query );