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

#include <SDL.h>

#include "codecs/mjpeg/multipartblock.h"

namespace Watcher
{

MultipartBlock::MultipartBlock(const ByteArray& bytes, size_t count) :
m_ContentLength(0u)
{
	size_t offset = 0u;
	std::string header;
	std::string headerValue;
	while (GetHeader(bytes, offset, header, headerValue))
	{
		if (header == "Content-Type")
		{
			m_ContentType = headerValue;
		}
		else if (header == "Content-Length")
		{
			m_ContentLength = std::stoul(headerValue);
		}
	}
	
	size_t dataSize = count - offset - 2; // -2: strip the "/r/n"
	if (dataSize > 0u)
	{
		m_Bytes.resize(dataSize);
		memcpy(&m_Bytes[0], &bytes[offset], dataSize);
	}
}

const std::string& MultipartBlock::GetType() const
{
	return m_ContentType;
}

bool MultipartBlock::IsValid() const
{
	return !m_ContentType.empty() && !m_Bytes.empty() && m_ContentLength > 0u && m_Bytes.size() == m_ContentLength;
}

bool MultipartBlock::GetHeader(const ByteArray& bytes, size_t& offset, std::string& header, std::string& headerValue)
{
	size_t numBytes = bytes.size();

	for (size_t i = offset; i < numBytes - 1; i++)
	{
		if (bytes[i] == '\r' && bytes[i+1] == '\n') // Line break.
		{
			std::string headerData = GetString(bytes, offset, i - offset);
			offset += i - offset + 2; // +2, jump over the \r\n.

			if (headerData.empty())
			{
				return false;
			}
			else
			{
				// Format for a HTTP header is "header: headerValue".
				size_t separator = headerData.find_first_of(": ");
				if (separator != std::string::npos)
				{
					header = headerData.substr(0, separator);
					headerValue = headerData.substr(separator + 2);
				}

				return true;
			}
		}
	}

	return false;
}

std::string MultipartBlock::GetString(const ByteArray& bytes, size_t offset, size_t count)
{
	std::string str;
	str.resize(count);
	memcpy(&str[0], &bytes[0] + offset, count * sizeof(uint8_t));
	return str;
}

const ByteArray& MultipartBlock::GetBytes() const
{
	return m_Bytes;
}

} // namespace Watcher