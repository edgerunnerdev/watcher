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

#include "tasks/searchtask.h"

namespace Watcher
{

SearchTask::SearchTask(const std::string& name) : 
Task(name)
{

}

SearchTask::~SearchTask() 
{

}

void SearchTask::Update(float delta)
{
    Task::Update(delta);
}

void SearchTask::Start()
{
    Task::Start();
}

void SearchTask::Stop()
{
    Task::Stop();
}


void SearchTask::AddQuery(const std::string& query)
{

}

void SearchTask::RemoveQuery(const std::string& query)
{

}

void SearchTask::OnQuery(const std::string& query)
{

}

void SearchTask::OnResult(const std::string& result, const DatabaseTime& time)
{

}

} // namespace Watcher
