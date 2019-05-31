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
#include <memory>
#include <thread>
#include <vector>
#include <network/network.h>

class IPGenerator;
using IPGeneratorUniquePtr = std::unique_ptr<IPGenerator>;

class HTTPScanner
{
public:
	HTTPScanner();
	~HTTPScanner();
	int Go( Network::IPAddress block, int numThreads, const std::vector< uint16_t >& ports );
	void Stop();
	bool IsScanning() const;
	int GetRemaining() const;

private:
	using ThreadVector = std::vector< std::thread >;
	ThreadVector m_Threads;
	std::atomic_int m_ActiveThreads;
	std::atomic_bool m_Stop;
	IPGeneratorUniquePtr m_pIPGenerator;
};
