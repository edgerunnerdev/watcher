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
#include <mutex>
#include <thread>
#include <vector>

#include "../watcher/plugin.h"
#include "network/network.h"
#include "coverage.h"

using CURL = void;
class IPGenerator;
using IPGeneratorUniquePtr = std::unique_ptr<IPGenerator>;

class PortScanner : public Plugin
{
	DECLARE_PLUGIN(PortScanner, 0, 1, 0);
public:
	PortScanner();
	virtual ~PortScanner();
	virtual bool Initialise(PluginMessageCallback pMessageCallback) override;
	virtual void OnMessageReceived(const nlohmann::json& message) override;
	virtual void DrawUI(ImGuiContext* pContext) override;

	void ScanNextBlock();

	int Go(Network::IPAddress block, int numThreads, const Network::PortVector& ports);
	void Stop();
	bool IsStopping() const;
	bool IsScanning() const;
	int GetRemaining() const;

private:
	static void ThreadMain(PortScanner* pPortScanner);
	void OnHTTPServerFound(const Network::IPAddress& address);

	void StartPortscan();
	void StopPortscan();

	PluginMessageCallback m_pMessageCallback;
	Coverage m_Coverage;
	int m_BlockIPsToScan;
	Network::IPAddress m_Block;

	using ThreadVector = std::vector< std::thread >;
	ThreadVector m_Threads;
	std::atomic_int m_ActiveThreads;
	std::atomic_bool m_Stop;
	IPGeneratorUniquePtr m_pIPGenerator;
	Network::PortVector m_Ports;
};