#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include "database/database.h"
#include "network/network.h"
#include "camera_scanner.h"
#include "geo_info.h"

#include "json.h"
using json = nlohmann::json;

namespace PortScanner
{
	class Coverage;
	using CoverageUniquePtr = std::unique_ptr< Coverage >;
}

class AtlasGrid;
class Configuration;
class CameraScanner;
class InternetScannerNmap;
class WebMasscanParser;
class InternetScannerZmap;
class InternetScannerBasic;
class PluginManager;
class WatcherRep;
struct SDL_Window;

using InternetScannerBasicUniquePtr = std::unique_ptr< InternetScannerBasic >;
using InternetScannerNmapUniquePtr = std::unique_ptr< InternetScannerNmap >;
using InternetScannerZmapUniquePtr = std::unique_ptr< InternetScannerZmap >;
using InternetScannerBasicVector = std::vector< InternetScannerBasicUniquePtr >;
using ThreadVector = std::vector< std::thread >;
using IPVector = std::vector< Network::IPAddress >;
using CameraScannerUniquePtr = std::unique_ptr< CameraScanner >;
using CameraScannerVector = std::vector< CameraScannerUniquePtr >;
using WatcherRepUniquePtr = std::unique_ptr< WatcherRep >;
using GeoInfoVector = std::vector< GeoInfo >;
using ConfigurationUniquePtr = std::unique_ptr< Configuration >;
using PluginManagerUniquePtr = std::unique_ptr< PluginManager >;

enum class InternetScannerMode
{
	None,
	Basic,
	Nmap,
	Zmap
};

class Watcher
{
public:
	Watcher( SDL_Window* pWindow, unsigned int scannerCount );
	~Watcher();
	void Update();
	bool IsActive() const;
	PortScanner::Coverage* GetPortScannerCoverage() const;
	Configuration* GetConfiguration() const;

	void OnWebServerFound( Network::IPAddress address );
	void OnCameraScanned( const CameraScanResult& result );
	void OnMessageReceived( const json& message );

	bool ConsumeCameraScannerQueue( Network::IPAddress& address );

	GeoInfoVector GetGeoInfos();

private:
	static void GeolocationRequestCallback( const Database::QueryResult& result, void* pData );
	static void PopulateCameraDetectionQueueCallback( const Database::QueryResult& result, void* pData );
	static void LoadGeoInfosCallback( const Database::QueryResult& result, void* pData );

	void PopulateCameraDetectionQueue();
	void InitialiseInternetScannerBasic( unsigned int scannerCount );
	void InitialiseNmap();
	void InitialiseZmap();
	void InitialiseCameraScanners( unsigned int scannerCount );
	void RestartCameraDetection();
	void InitialiseGeolocation();
	void LoadGeoInfos();
	void AddGeoInfo( const json& message );

	bool m_Active;
	ThreadVector m_InternetScannerBasicThreads;
	InternetScannerBasicVector m_InternetScannerBasic;
	InternetScannerMode m_WebServerScannerMode;
	Database::DatabaseUniquePtr m_pDatabase;

	std::mutex m_CameraScannerQueueMutex;
	IPVector m_CameraScannerQueue;
	CameraScannerVector m_CameraScanners;
	ThreadVector m_CameraScannerThreads;
	std::mutex m_CameraScanResultsMutex;
	CameraScanResultList m_CameraScanResults;

	std::mutex m_GeoInfoMutex;
	GeoInfoVector m_GeoInfos;

	WatcherRepUniquePtr m_pRep;
	ConfigurationUniquePtr m_pConfiguration;

	InternetScannerNmapUniquePtr m_pInternetScannerNmap;
	std::thread m_InternetScannerNmapThread;

	InternetScannerZmapUniquePtr m_pInternetScannerZmap;
	std::thread m_InternetScannerZmapThread;

	PortScanner::CoverageUniquePtr m_pPortScannerCoverage;
	bool m_PortScannerCoverageOpen;

	PluginManagerUniquePtr m_pPluginManager;
};

extern Watcher* g_pWatcher;

inline bool Watcher::IsActive() const
{
	return m_Active;
}

inline Configuration* Watcher::GetConfiguration() const
{
	return m_pConfiguration.get();
}

inline GeoInfoVector Watcher::GetGeoInfos()
{
	std::scoped_lock< std::mutex > lock( m_GeoInfoMutex );
	GeoInfoVector v = m_GeoInfos;
	return v;
}

inline PortScanner::Coverage* Watcher::GetPortScannerCoverage() const
{
	return m_pPortScannerCoverage.get();
}