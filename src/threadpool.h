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

#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <functional>
#include <vector>

class ThreadPool
{
public:
	ThreadPool(int numThreads);
	~ThreadPool();

	using Job = std::function<void()>;
	void Queue(Job job);

private:
	static void ThreadMain(ThreadPool* pThreadPool);

	using ThreadVector = std::vector< std::thread >;
	ThreadVector m_Threads;
	std::mutex m_JobMutex;
	std::atomic_bool m_Stop;
	std::condition_variable m_Condition;
	std::deque<Job> m_Jobs;
};
