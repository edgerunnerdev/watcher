#pragma once

#include <mutex>
#include <string>

#include "network/network.h"

struct sqlite3;
using CURL = void;

class GeoScanner
{
public:
	GeoScanner();
	~GeoScanner();
	void Update();
	void QueueScan( Network::IPAddress address );
	size_t GetQueueSize() const;

private:
	void PopulateQueueFromDatabase();
	void PopulateResultsFromDatabase();
	std::string m_Data;

	mutable std::mutex m_QueueMutex;
	std::vector< Network::IPAddress > m_Queue;
	sqlite3* m_pDatabase;
	CURL* m_pCurlHandle;
};