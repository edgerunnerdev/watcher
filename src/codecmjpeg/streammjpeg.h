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

#pragma once

#include <cstddef>
#include <string>

#include "../watcher/plugin.h"
#include "network/network.h"

using CURL = void;

class StreamMJPEG 
{
public:
	StreamMJPEG(const std::string& url);
	~StreamMJPEG();

	enum class State
	{
		Initialising,
		Streaming,
		Error,
		Terminated
	};

	enum class Error
	{
		NoError,
		Timeout
	};

	State GetState() const;

	using Id = unsigned int;
	Id GetId() const;

private:
	static size_t WriteHeaderCallback(void* pContents, size_t size, size_t nmemb, void* pUserData);
	static size_t WriteResponseCallback(void* pContents, size_t size, size_t nmemb, void* pUserData);

	Error m_Error;
	State m_State;
	Id m_Id;
	CURL* m_pCurlHandle;
	std::vector<uint8_t> m_HeaderBuffer;
	std::vector<uint8_t> m_ResponseBuffer;
	std::string m_Url;
	std::string m_MultiPartDivider;
};
