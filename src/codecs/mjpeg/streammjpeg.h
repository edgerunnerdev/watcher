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

#include <array>
#include <cstddef>
#include <string>

#include "codecs/stream.h"
#include "codecs/mjpeg/multipartblock.h"

using CURL = void;
using ByteArray = std::vector<uint8_t>;

namespace Watcher
{

class StreamMJPEG : public Stream
{
public:
	StreamMJPEG(const std::string& url, uint32_t textureId);
    virtual ~StreamMJPEG() override;
    virtual void Update() override;
    virtual const std::string& GetError() const override;

private:
	static size_t WriteHeaderCallback(void* pContents, size_t size, size_t nmemb, void* pUserData);
	static size_t WriteResponseCallback(void* pContents, size_t size, size_t nmemb, void* pUserData);

	static bool IsContentTypeHeader(const std::string& header);
	static bool IsContentTypeMultipart(const std::string& header);
	static bool ExtractMultipartBoundary(const std::string& header, std::string& result);
	static void ProcessMultipartContent(StreamMJPEG* pStream);
	static size_t FindInStream(StreamMJPEG* pStream, size_t offset, const std::string& toFind);

    enum class Error
    {
        NoError,
        InvalidBlock,
        UnsupportedContentType,
        UnknownBoundary,
        DecodingError,
        UnknownError,
        Timeout,

        Count
    };

	void SetError(Error error);
	Error CopyFrame(const MultipartBlock& block);
	
	Error m_Error; // Do not set directly, use SetError().
    std::array<std::string, static_cast<size_t>(Error::Count)> m_ErrorText;
    
	State m_State;
	CURL* m_pCurlMultiHandle;
	CURL* m_pCurlHandle;
	ByteArray m_HeaderBuffer;
	ByteArray m_ResponseBuffer;
	std::string m_MultipartBoundary;
	bool m_FrameAvailable;

	MultipartBlock* m_pMultipartBlock;
};

} // namespace Watcher
