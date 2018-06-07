#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include "network/network.h"
#include "camera_detection_rule.h"

class IPGenerator;
class CameraDetectionRule;
class CameraScanner;
class Scanner;
struct sqlite3;

using ScannerUniquePtr = std::unique_ptr< Scanner >;
using IPGeneratorUniquePtr = std::unique_ptr< IPGenerator >;
using ScannerVector = std::vector< ScannerUniquePtr >;
using ThreadVector = std::vector< std::thread >;
using IPVector = std::vector< Network::IPAddress >;
using CameraDetectionRuleVector = std::vector< CameraDetectionRule >;
using CameraScannerUniquePtr = std::unique_ptr< CameraScanner >;
using CameraScannerVector = std::vector< CameraScannerUniquePtr >;

class Watcher
{
public:
	Watcher( unsigned int scannerCount );
	~Watcher();
	void Update();
	bool IsActive() const;
	IPGenerator* GetIPGenerator() const;
	const CameraDetectionRuleVector& GetCameraDetectionRules() const;
	void OnWebServerFound( Network::IPAddress address );
	void OnWebServerAddedFromDatabase( Network::IPAddress address );
	bool ConsumeCameraScannerQueue( Network::IPAddress& address );

private:
	void WriteConfig();
	void ReadConfig();
	void LoadCameraDetectionRules();
	void PopulateCameraDetectionQueue();
	void InitialiseWebServerScanners( unsigned int scannerCount );
	void InitialiseCameraScanners( unsigned int scannerCount );

	bool m_Active;
	ThreadVector m_ScannerThreads;
	ScannerVector m_Scanners;
	IPGeneratorUniquePtr m_pIPGenerator;
	sqlite3* m_pDatabase;

	Network::IPAddress m_ConfigInitialIP;
	CameraDetectionRuleVector m_CameraDetectionRules;
	std::mutex m_CameraScannerQueueMutex;
	IPVector m_CameraScannerQueue;
	ThreadVector m_CameraScannerThreads;
	CameraScannerVector m_CameraScanners;
};

extern Watcher* g_pWatcher;

inline bool Watcher::IsActive() const
{
	return m_Active;
}

inline IPGenerator* Watcher::GetIPGenerator() const
{
	return m_pIPGenerator.get();
}

inline const CameraDetectionRuleVector& Watcher::GetCameraDetectionRules() const
{
	return m_CameraDetectionRules;
}