#include <SDL.h>
#include "multipartblock.h"

MultipartBlock::MultipartBlock(const ByteArray& bytes, size_t offset, size_t count) :
m_ContentLength(0u)
{
	std::string header;
	std::string headerValue;
	while (GetHeader(bytes, offset, header, headerValue))
	{
		if (header == "Content-Type")
		{
			m_ContentType = headerValue;
		}
	}
	int a = 0;
}

const std::string& MultipartBlock::GetType() const
{
	return m_ContentType;
}

bool MultipartBlock::IsValid() const
{
	return m_Bytes.size() == m_ContentLength;
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