#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include "network/network.h"

class Scanner;
using ScannerUniquePtr = std::unique_ptr< Scanner >;
class IPGenerator;
using IPGeneratorUniquePtr = std::unique_ptr< IPGenerator >;
using ScannerVector = std::vector< ScannerUniquePtr >;
using ThreadVector = std::vector< std::thread >;
using IPVector = std::vector< Network::IPAddress >;
struct sqlite3;

class Watcher
{
public:
	Watcher( unsigned int scannerCount );
	~Watcher();
	void Update();
	bool IsActive() const;
	IPGenerator* GetIPGenerator() const;
	void AddEntry( Network::IPAddress address );
	sqlite3* GetDatabase() const;

private:
	void WriteConfig();
	void ReadConfig();

	bool m_Active;
	ThreadVector m_ScannerThreads;
	ScannerVector m_Scanners;
	IPGeneratorUniquePtr m_pIPGenerator;
	IPVector m_KnownServers;
	std::mutex m_KnownServersMutex;
	sqlite3* m_pDatabase;

	Network::IPAddress m_ConfigInitialIP;
};

inline bool Watcher::IsActive() const
{
	return m_Active;
}

inline IPGenerator* Watcher::GetIPGenerator() const
{
	return m_pIPGenerator.get();
}