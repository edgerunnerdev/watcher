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
#include <cstdint>
#include <mutex>
#include <vector>
#include <network/network.h>

class IPGenerator
{
public:
	IPGenerator() {}
	IPGenerator( const Network::IPAddress& address );

	bool GetNext( Network::IPAddress& address );
	int GetRemaining() const;

private:
	std::mutex m_Mutex;
	using PossibleIPs = std::vector< uint32_t >;
	PossibleIPs m_PossibleIPs;
	std::atomic_int m_RemainingIPs;
};
