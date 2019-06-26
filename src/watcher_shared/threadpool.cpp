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

#include "threadpool.h"

ThreadPool::ThreadPool(int numThreads) :
m_Stop(false)
{
	for (int i = 0; i < numThreads; ++i)
	{
		m_Threads.emplace_back(&ThreadMain, this);
	}
}

ThreadPool::~ThreadPool()
{
	m_Stop = true;

	for (auto& thread : m_Threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
}

void ThreadPool::Queue(Job job)
{
	std::lock_guard<std::mutex> lock(m_JobMutex);
	m_Jobs.push_back(job);
	m_Condition.notify_one();
}

void ThreadPool::ThreadMain(ThreadPool* pThreadPool)
{
	while (pThreadPool->m_Stop == false)
	{
		Job job;
		{
			std::unique_lock<std::mutex> lock(pThreadPool->m_JobMutex);
			pThreadPool->m_Condition.wait(lock, [pThreadPool]() { return !pThreadPool->m_Jobs.empty(); });
			job = pThreadPool->m_Jobs.front();
			pThreadPool->m_Jobs.pop_front();
		}
		job();
	}
}