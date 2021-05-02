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

#include <cassert>
#include <sstream>
#include "network.h"

namespace Network
{

	//-----------------------------------------------------------------------------
	// IPAddress
	//-----------------------------------------------------------------------------

	IPAddress::IPAddress() :
		m_Host(0),
		m_Port(0),
		m_Block(0)
	{

	}

	IPAddress::IPAddress(const std::vector< int >& host, uint16_t port)
	{
		SetHost(host);
		SetPort(port);
		SetBlock(0);
	}

	IPAddress::IPAddress(unsigned int host, uint16_t port)
	{
		m_Host = host;
		SetPort(port);
		SetBlock(0);
	}

	IPAddress::IPAddress(const std::string& address)
	{
#ifdef _WIN32
#define sscanf sscanf_s
#endif

		assert(IPAddress::IsValid(address));

		std::vector< int > v = { 0, 0, 0, 0 };
		int port = 0;
		int block = 0;
		if (sscanf(address.c_str(), "%d.%d.%d.%d:%d", &v[0], &v[1], &v[2], &v[3], &port) == 5)
		{
			SetHost(v);
			SetPort(port);
			SetBlock(0);
		}
		else if (sscanf(address.c_str(), "%d.%d.%d.%d/%d", &v[0], &v[1], &v[2], &v[3], &block) == 5)
		{
			SetHost(v);
			SetPort(0);
			SetBlock(block);
		}
		else if (sscanf(address.c_str(), "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) == 4)
		{
			SetHost(v);
			SetPort(0);
			SetBlock(0);
		}
		else
		{
			// Invalid format
			assert(false);
		}
#ifdef _WIN32
#undef sscanf
#endif
	}

	bool IPAddress::IsValid(const std::string& address)
	{
#ifdef _WIN32
#define sscanf sscanf_s
#endif
		std::vector< int > v = { 0, 0, 0, 0 };
		int port = 0;
		int block = 0;
		if (sscanf(address.c_str(), "%d.%d.%d.%d:%d", &v[0], &v[1], &v[2], &v[3], &port) == 5 ||
			sscanf(address.c_str(), "%d.%d.%d.%d/%d", &v[0], &v[1], &v[2], &v[3], &block) == 5 ||
			sscanf(address.c_str(), "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]) == 4)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (v[i] < 0 || v[i] > 255)
				{
					return false;
				}
			}

			if (port < 0 || port > 65535)
			{
				return false;
			}
			else if ((block > 0 && block < 8) || block > 24)
			{
				return false;
			}

			return true;
		}

		return false;

#ifdef _WIN32
#undef sscanf
#endif
	}

	void IPAddress::SetHost(const std::vector< int >& host)
	{
		// Convert the vector into a single unsigned integer in host order.
		assert(host.size() == 4);
		m_Host = (host[0] << 24) | (host[1] << 16) | (host[2] << 8) | (host[3]);
	}

	uint32_t IPAddress::GetHost() const
	{
		return m_Host;
	}

	void IPAddress::SetPort(uint16_t port)
	{
		m_Port = port;
	}

	uint16_t IPAddress::GetPort() const
	{
		return m_Port;
	}

	void IPAddress::SetBlock(uint8_t block)
	{
		assert(block == 0u || m_Port == 0u);
		m_Block = block;
	}

	uint8_t IPAddress::GetBlock() const
	{
		return m_Block;
	}

	std::string IPAddress::ToString() const
	{
		std::stringstream ss;
		ss << GetHostAsString();

		if (m_Port != 0u)
		{
			ss << ":" << m_Port;
		}

		return ss.str();
	}

	std::string IPAddress::GetHostAsString() const
	{
		std::stringstream ss;
		ss <<
			(m_Host >> 24) << "." <<
			((m_Host >> 16) & 0xFF) << "." <<
			((m_Host >> 8) & 0xFF) << "." <<
			(m_Host & 0xFF);
		return ss.str();
	}

}