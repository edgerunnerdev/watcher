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
#include "httpscanner/coverage.h"

class HTTPScanner;
using HTTPScannerUniquePtr = std::unique_ptr< HTTPScanner >;
using CURL = void;

class Portscanner : public Plugin
{
	DECLARE_PLUGIN(Portscanner, 0, 1, 0);
public:
	Portscanner();
	virtual ~Portscanner();
	virtual bool Initialise(PluginMessageCallback pMessageCallback) override;
	virtual void OnMessageReceived(const nlohmann::json& message) override;
	virtual void DrawUI(ImGuiContext* pContext) override;

	void OnHTTPServerFound(const Network::IPAddress& address);
	void ScanNextBlock();

private:
	void StartPortscan();
	void StopPortscan();

	PluginMessageCallback m_pMessageCallback;
	Coverage m_Coverage;
	HTTPScannerUniquePtr m_pHTTPScanner;
	int m_BlockIPsToScan;
};