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

#include <iostream>
#include <stdio.h>
#include <imgui/imgui.h>
#include "ipgenerator.h"
#include "portprobe.h"
#include "portscanner.h"

IMPLEMENT_PLUGIN(PortScanner)

PortScanner::PortScanner()
{
	m_WantedThreads = 100;
	m_BlockIPsToScan = 0;
	m_ActiveThreads = 0;
	m_Stop = false;
}

PortScanner::~PortScanner()
{
	Stop();

	for (auto& thread : m_Threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
}

void PortScanner::ThreadMain(PortScanner* pPortScanner)
{
	PortProbe probe;
	Network::IPAddress address;
	while (pPortScanner->m_pIPGenerator->GetNext(address))
	{
		for (uint16_t port : pPortScanner->m_Ports)
		{
			address.SetPort(port);
			if (probe.Probe(address) == PortProbe::Result::Open && pPortScanner->IsStopping() == false)
			{
				pPortScanner->OnHTTPServerFound(address);
			}

			if (pPortScanner->IsStopping())
			{
				pPortScanner->m_ActiveThreads--;
				return;
			}
		}
	}

	pPortScanner->m_ActiveThreads--;
}

bool PortScanner::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;
	m_Coverage.Read();
	return true;
}

void PortScanner::OnMessageReceived(const nlohmann::json& message)
{
	//if ( message[ "type" ] == "geolocation_request" )
	//{
	//	{
	//		std::lock_guard< std::mutex > lock( m_AccessMutex );
	//		std::string address = message[ "address" ];
	//		m_Queue.emplace_back( address );
	//	}

	//	ConsumeQueue();
	//}
}

void PortScanner::StartPortscan()
{
	if (IsScanning() == false)
	{
		ScanNextBlock();
	}
}

void PortScanner::StopPortscan()
{
	if (IsScanning())
	{
		Stop();
	}
}

void PortScanner::ScanNextBlock()
{
	if (m_Coverage.GetNextBlock(m_Block))
	{
		m_BlockIPsToScan = Go(m_Block, Network::PortVector{ 80, 81, 8080 });
	}
}

// Must be MT safe, as it gets called from the worker threads.
void PortScanner::OnHTTPServerFound(const Network::IPAddress& address)
{
	std::string logText = "Found HTTP server: " + address.ToString();
	json message =
	{
		{ "type", "log" },
		{ "level", "info" },
		{ "plugin", "portscanner" },
		{ "message", logText }
	};
	m_pMessageCallback(message);

	message =
	{
		{ "type", "http_server_found" },
		{ "address", address.ToString() }
	};
	m_pMessageCallback(message);
}

void PortScanner::DrawUI(ImGuiContext* pContext)
{
	ImGui::SetCurrentContext(pContext);

	if (ImGui::CollapsingHeader("Port scanner", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (IsScanning())
		{
			float r = 1.0f - static_cast<float>(GetRemaining()) / static_cast<float>(m_BlockIPsToScan);

			std::string text = "Block completion (" + m_Block.ToString() + "/8):";
			ImGui::Text(text.c_str());
			ImGui::ProgressBar(r);

			if (ImGui::Button("Stop scan"))
			{
				Stop();
			}
		}
		else
		{
			if (ImGui::Button("Begin scan"))
			{
				ScanNextBlock();
			}
			
			ImGui::SliderInt("Threads", &m_WantedThreads, 20, 200);    
		}
	}
}

int PortScanner::Go(Network::IPAddress block, const Network::PortVector& ports)
{
	//SDL_assert( m_ActiveThreads == 0 );
	m_ActiveThreads = m_WantedThreads;
	m_Stop = false;
	m_Ports = ports;
	m_pIPGenerator = std::make_unique<IPGenerator>(block);
	int remaining = m_pIPGenerator->GetRemaining();

	for (int i = 0; i < m_WantedThreads; ++i)
	{
		m_Threads.emplace_back(&PortScanner::ThreadMain, this);
	}

	return remaining;
}

bool PortScanner::IsScanning() const
{
	return m_ActiveThreads > 0;
}

void PortScanner::Stop()
{
	m_Stop = true;
}

bool PortScanner::IsStopping() const
{
	return m_Stop;
}

int PortScanner::GetRemaining() const
{
	return m_pIPGenerator ? m_pIPGenerator->GetRemaining() : 0;
}
