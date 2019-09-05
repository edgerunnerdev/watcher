#include <SDL.h>
#include "multipartblock.h"

MultipartBlock::MultipartBlock(const ByteArray& bytes, size_t offset, size_t count) :
m_ExpectedBytes(0u)
{
	//m_Bytes.resize(count);
	//SDL_assert(offset + count < bytes.size());
	//if (offset + count < bytes.size())
	//{
	//	memcpy(&m_Bytes[0], &bytes[offset], count * sizeof(uint8_t));
	//}

	std::string header;
	std::string headerValue;
	while (GetHeader(bytes, offset, header, headerValue))
	{

	}
}

const std::string& MultipartBlock::GetType() const
{
	return m_Type;
}

bool MultipartBlock::IsValid() const
{
	return m_Bytes.size() == m_ExpectedBytes;
}

bool MultipartBlock::GetHeader(const ByteArray& bytes, size_t& offset, std::string& header, std::string& headerValue)
{
	size_t numBytes = bytes.size();

	for (size_t i = offset; i < numBytes - 1; i++)
	{
		if (bytes[i] == '\r' && bytes[i+1] == '\n') // Line break
		{
			std::string headerData = GetString(bytes, offset, i - offset);
			return true;
		}
	}

	return false;
}

std::string GetString(const ByteArray& bytes, size_t offset, size_t count)
{
	std::string str;
	str.resize(count + 1);
	for (size_t i = 0; i < count; ++i)
	{
		str[i] = static_cast<char>(bytes[offset + i]);
	}
	return str;
}