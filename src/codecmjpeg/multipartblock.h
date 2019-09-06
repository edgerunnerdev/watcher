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

#include <string>
#include <vector>

using ByteArray = std::vector<uint8_t>;

class MultipartBlock
{
public:
	MultipartBlock(const ByteArray& bytes, size_t count);

	const std::string& GetType() const;
	bool IsValid() const;
	const ByteArray& GetBytes() const;

private:
	bool GetHeader(const ByteArray& bytes, size_t& offset, std::string& header, std::string& headerValue);
	std::string GetString(const ByteArray& bytes, size_t offset, size_t count);

	std::string m_ContentType;
	size_t m_ContentLength;
	ByteArray m_Bytes;
};