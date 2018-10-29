#pragma once

#include <list>
#include <vector>

#include "network/network.h"
#include "camera_detection_rule.h"

using CameraDetectionRuleVector = std::vector< CameraDetectionRule >;

struct CameraScanResult
{
	bool isCamera;
	Network::IPAddress address;
	std::string title;
};
using CameraScanResultVector = std::vector< CameraScanResult >;
using CameraScanResultList = std::list< CameraScanResult >;

class CameraScanner
{
public:
	CameraScanner();

	CameraScanResult Scan( Network::IPAddress address );

	int GetRuleCount() const;
	void SetTitle( const std::string& title );

private:
	void LoadCameraDetectionRules();
	bool EvaluateDetectionRules( const std::string& url ) const;

	std::string m_Title;
	CameraDetectionRuleVector m_CameraDetectionRules;
};

inline int CameraScanner::GetRuleCount() const
{
	return m_CameraDetectionRules.size();
}