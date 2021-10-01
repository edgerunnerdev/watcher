#include <string>

namespace Encoding
{

// Convert a wide Unicode string to an UTF8 string
std::string UTF8Encode(const std::wstring& wstr);

// Convert an UTF8 string to a wide Unicode String
std::wstring UTF8Decode(const std::string& str);

std::string URLEncode(const std::string& str);

} // namespace Encoding