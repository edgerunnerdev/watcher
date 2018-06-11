#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include "network/network.h"
#include "camera_scanner.h"
#include "geo_info.h"
#include "geo_scanner.h"

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
using GeoScannerUniquePtr = std::unique_ptr< GeoScanner >;
using GeoInfoVector = std::vector< GeoInfo >;

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
	void OnGeoInfoAdded( const GeoInfo& geoInfo );

	bool ConsumeCameraScannerQueue( Network::IPAddress& address );

	const GeoInfoVector& GetGeoInfos() const;

private:
	void WriteConfig();
	void ReadConfig();
	void PopulateCameraDetectionQueue();
	void InitialiseWebServerScanners( unsigned int scannerCount );
	void InitialiseCameraScanner();
	void InitialiseGeoScanner();
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

	std::thread m_GeoScannerThread;
	GeoScannerUniquePtr m_pGeoScanner;

	std::mutex m_GeoInfoMutex;
	GeoInfoVector m_GeoInfos;

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

inline const GeoInfoVector& Watcher::GetGeoInfos() const
{
	return m_GeoInfos;
}