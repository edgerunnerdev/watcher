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
}

const std::string& MultipartBlock::GetType() const
{
	return m_Type;
}

bool MultipartBlock::IsValid() const
{
	return m_Bytes.size() == m_ExpectedBytes;
}
