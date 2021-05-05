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

#pragma once

#include <vector>
#include "network/network.h"

namespace Watcher
{

class Configuration
{
public:
	Configuration();
	~Configuration();

	Network::IPAddress GetWebScannerStartAddress() const;
	void SetWebScannerStartAddress( Network::IPAddress address );

	int GetWebScannerRate() const;
	void SetWebScannerRate( int value );

	const Network::PortVector& GetWebScannerPorts() const;
	void SetWebScannerPorts( const Network::PortVector& ports );

private:
	void Save();
	void Load();
	void UseDefaults();

	Network::IPAddress m_StartAddress;
	int m_Rate;
	Network::PortVector m_Ports;
};

} // namespace Watcher
