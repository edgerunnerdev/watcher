///////////////////////////////////////////////////////////////////////////////
// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <filesystem>
#include "sqlite/sqlite3.h"
#include "configuration.h"
#include "database.h"
#include "log.h"
#include "watcher.h"

namespace Watcher
{

Database::Database() :
    m_pDatabase(nullptr),
    m_RunThread(true),
    m_FreshlyCreated(false)
{

}

Database::~Database()
{
    m_RunThread = false;
    if (m_Thread.joinable())
    {
        m_Thread.join();
    }

    if (m_pDatabase != nullptr)
    {
        sqlite3_close(m_pDatabase);
    }
}

bool Database::Initialise()
{   
    std::filesystem::path filename = g_pWatcher->GetConfiguration()->GetStoragePath() / "watcher.db";
    const bool createTables = (std::filesystem::exists(filename) == false);
    if (sqlite3_open(filename.string().c_str(), &m_pDatabase) != SQLITE_OK)
    {
        Log::Error("Couldn't open database '%s'", filename.string().c_str());
        return false;
    }
    else
    {
        Log::Info("Opened database '%s'.", filename.string().c_str());
        m_Thread = std::thread(sThreadMain, this);

        if (createTables)
        {
            CreateTables();
            m_FreshlyCreated = true;
        }

        return true;
    }
}

bool Database::IsFreshlyCreated() const
{
    return m_FreshlyCreated;
}

void Database::CreateTables()
{
    CreateCamerasTable();
    CreateGeolocationTable();
    CreateSearchQueriesTable();
}

void Database::CreateCamerasTable()
{
    std::string query(
        "CREATE TABLE 'Cameras' ("
            "'Id' INTEGER UNIQUE, "
            "'URL' TEXT NOT NULL UNIQUE, "
            "'IP' TEXT NOT NULL, "
            "'Port' INTEGER NOT NULL, "
            "'Title' TEXT, "
            "'GeolocationId' INTEGER, "
            "'Date' TEXT NOT NULL, "
            "'Type' INTEGER NOT NULL, "
            "PRIMARY KEY('Id' AUTOINCREMENT) "
        "); "
    );
    PreparedStatement statement(this, query);
    statement.ExecuteBlocking();
}

void Database::CreateGeolocationTable()
{
    std::string query(
        "CREATE TABLE 'Geolocation' ("
            "'Id' INTEGER UNIQUE, "
            "'IP' TEXT NOT NULL UNIQUE, "
            "'City' TEXT, "
            "'Region' TEXT, "
            "'Country' TEXT, "
            "'Organisation' TEXT, "
            "'Latitude' REAL, "
            "'Longitude' REAL "
        "); "
    );
    PreparedStatement statement(this, query);
    statement.ExecuteBlocking();
}

void Database::CreateSearchQueriesTable()
{
    std::string query(
        "CREATE TABLE 'SearchQueries' ("
            "'Engine' TEXT NOT NULL, "
            "'Query' TEXT NOT NULL, "
            "'Date' TEXT"
        "); "
    );
    PreparedStatement statement(this, query);
    statement.ExecuteBlocking();
}

void Database::Execute(PreparedStatement statement)
{
    std::lock_guard< std::mutex > pendingLock(m_PendingStatementsMutex);
    m_PendingStatements.push_back(statement);
}

void Database::sThreadMain(Database* pDatabase)
{
    while (pDatabase->m_RunThread)
    {
        pDatabase->ConsumeStatements();
        pDatabase->ExecuteActiveStatements();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void Database::ConsumeStatements()
{
    std::lock_guard< std::mutex > pendingLock(m_PendingStatementsMutex);
    std::lock_guard< std::mutex > activeLock(m_ActiveStatementsMutex);
    for (const PreparedStatement& statement : m_PendingStatements)
    {
        m_ActiveStatements.push_back(statement);
    }
    m_PendingStatements.clear();
}

void Database::ExecuteActiveStatements()
{
    std::lock_guard< std::mutex > activeLock(m_ActiveStatementsMutex);
    if (m_ActiveStatements.empty() == false)
    {
        BlockingNonQuery("BEGIN TRANSACTION;");
        for (PreparedStatement& statement : m_ActiveStatements)
        {
            statement.ExecuteBlocking();
        }
        m_ActiveStatements.clear();
        BlockingNonQuery("COMMIT;");
    }
}

void Database::BlockingNonQuery(const std::string& query)
{
    char* pError = nullptr;
    while (1)
    {
        int rc = sqlite3_exec(m_pDatabase, query.c_str(), nullptr, 0, &pError);
        if (rc != SQLITE_OK && rc != SQLITE_BUSY)
        {
            Log::Error("SQL query error: %s", pError);
            sqlite3_free(pError);
        }
        else if (rc == SQLITE_OK)
        {
            break;
        }
    }
}

} // namespace Watcher
