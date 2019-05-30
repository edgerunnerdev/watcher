#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include <SDL.h>

#include "database/database.h"
#include "network/network.h"
#include "camera_scanner.h"
#include "geo_info.h"

#include "json.h"
using json = nlohmann::json;

class Configuration;
class CameraScanner;
class InternetScannerNmap;
class WebMasscanParser;
class InternetScannerZmap;
class InternetScannerBasic;
class PluginManager;
class WatcherRep;
struct SDL_Window;

using ThreadVector = std::vector< std::thread >;
using IPVector = std::vector< Network::IPAddress >;
using CameraScannerUniquePtr = std::unique_ptr< CameraScanner >;
using CameraScannerVector = std::vector< CameraScannerUniquePtr >;
using WatcherRepUniquePtr = std::unique_ptr< WatcherRep >;
using GeoInfoVector = std::vector< GeoInfo >;
using ConfigurationUniquePtr = std::unique_ptr< Configuration >;
using PluginManagerUniquePtr = std::unique_ptr< PluginManager >;


class Watcher
{
public:
	Watcher( SDL_Window* pWindow, unsigned int scannerCount );
	~Watcher();
	void ProcessEvent( const SDL_Event& event );
	void Update();
	bool IsActive() const;
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
	void InitialiseCameraScanners( unsigned int scannerCount );
	void RestartCameraDetection();
	void InitialiseGeolocation();
	void LoadGeoInfos();
	void AddGeoInfo( const json& message );

	bool m_Active;
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
	std::lock_guard< std::mutex > lock( m_GeoInfoMutex );
	GeoInfoVector v = m_GeoInfos;
	return v;
}
