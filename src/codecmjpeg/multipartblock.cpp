//#include <string>
//#include <vector>
//
//using ByteArray = std::vector<uint8_t>;
//
//class MultipartBlock
//{
//public:
//	MultipartBlock();
//
//	void AddBytes(const ByteArray& bytes);
//	const ByteArray& GetBytes() const;
//
//	const std::string& GetType();
//	size_t GetTotalBytes() const;
//	size_t GetRemainingBytes() const;
//	bool IsComplete() const;
//
//private:
//	std::string m_Type;
//	size_t m_TotalBytes;
//	size_t m_RemainingBytes;
//	ByteArray m_Bytes;
//};