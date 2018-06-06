#pragma once

#include "network/network.h"

class CameraScanner
{
public:
	CameraScanner();

	bool Scan( Network::IPAddress address );

	void SetTitle( const std::string& title );

private:
	std::string m_Title;
	//CameraDetectionRuleVector m_Rules;
};