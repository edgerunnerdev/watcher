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
#include "httpscanner/httpscanner.h"
#include "portscanner.h"

IMPLEMENT_PLUGIN(Portscanner)

Portscanner::Portscanner()
{
	m_pHTTPScanner = std::make_unique<HTTPScanner>();
	m_BlockIPsToScan = 0;
}

Portscanner::~Portscanner()
{
	m_pHTTPScanner->Stop();
}

bool Portscanner::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;
	m_Coverage.Read();
	return true;
}

void Portscanner::OnMessageReceived(const nlohmann::json& message)
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

void Portscanner::StartPortscan()
{
	if (m_pHTTPScanner->IsScanning() == false)
	{
		ScanNextBlock();
	}
}

void Portscanner::StopPortscan()
{
	if (m_pHTTPScanner->IsScanning())
	{
		m_pHTTPScanner->Stop();
	}
}

void Portscanner::ScanNextBlock()
{
	Network::IPAddress address;
	if (m_Coverage.GetNextBlock(address))
	{
		m_BlockIPsToScan = m_pHTTPScanner->Go(address, 20, Network::PortVector{ 80, 81, 8080 });
	}
}

void Portscanner::OnHTTPServerFound(const Network::IPAddress& address)
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
}

void Portscanner::DrawUI(ImGuiContext* pContext)
{
	ImGui::SetCurrentContext(pContext);

	if (ImGui::CollapsingHeader("Portscanner", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_pHTTPScanner->IsScanning())
		{
			float r = static_cast<float>(m_pHTTPScanner->GetRemaining()) / static_cast<float>(m_BlockIPsToScan);
			ImGui::Text("Block completion:");
			ImGui::ProgressBar(r);

			if (ImGui::Button("Stop scan"))
			{
				m_pHTTPScanner->Stop();
			}
		}
		else if (ImGui::Button("Begin scan"))
		{
			ScanNextBlock();
		}
	}
}
