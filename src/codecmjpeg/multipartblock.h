#include <string>
#include <vector>

using ByteArray = std::vector<uint8_t>;

class MultipartBlock
{
public:
	MultipartBlock(const ByteArray& bytes, size_t offset, size_t count);

	const std::string& GetType() const;
	bool IsValid() const;

private:
	std::string m_Type;
	size_t m_ExpectedBytes;
	ByteArray m_Bytes;
};