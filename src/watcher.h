#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include "network/network.h"
#include "camera_scanner.h"

class IPGenerator;
class CameraScanner;
class Scanner;
class WatcherRep;
struct sqlite3;
struct SDL_Window;

using ScannerUniquePtr = std::unique_ptr< Scanner >;
using IPGeneratorUniquePtr = std::unique_ptr< IPGenerator >;
using ScannerVector = std::vector< ScannerUniquePtr >;
using ThreadVector = std::vector< std::thread >;
using IPVector = std::vector< Network::IPAddress >;
using CameraScannerUniquePtr = std::unique_ptr< CameraScanner >;
using WatcherRepUniquePtr = std::unique_ptr< WatcherRep >;

class Watcher
{
public:
	Watcher( SDL_Window* pWindow, unsigned int scannerCount );
	~Watcher();
	void Update();
	bool IsActive() const;
	IPGenerator* GetIPGenerator() const;
	void OnWebServerFound( Network::IPAddress address );
	void OnWebServerAddedFromDatabase( Network::IPAddress address );
	void OnCameraScanned( const CameraScanResult& result );
	bool ConsumeCameraScannerQueue( Network::IPAddress& address );

private:
	void WriteConfig();
	void ReadConfig();
	void PopulateCameraDetectionQueue();
	void InitialiseWebServerScanners( unsigned int scannerCount );
	void InitialiseCameraScanner();
	void RestartCameraDetection();
	void ExecuteDatabaseQuery( const std::string& query );
	void ProcessDatabaseQueryQueue();

	bool m_Active;
	ThreadVector m_ScannerThreads;
	ScannerVector m_Scanners;
	IPGeneratorUniquePtr m_pIPGenerator;
	sqlite3* m_pDatabase;

	Network::IPAddress m_ConfigInitialIP;
	std::mutex m_CameraScannerQueueMutex;
	IPVector m_CameraScannerQueue;
	std::thread m_CameraScannerThread;
	CameraScannerUniquePtr m_pCameraScanner;
	std::mutex m_CameraScanResultsMutex;
	CameraScanResultList m_CameraScanResults;

	std::thread::id m_MainThreadID;
	std::mutex m_QueryQueueMutex;
	std::vector< std::string > m_QueryQueue;

	WatcherRepUniquePtr m_pRep;
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