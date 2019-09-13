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

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <curl/curl.h>
#include <GL/gl.h>
#include <SDL.h>
#include <SDL_image.h>

#include "streammjpeg.h"

static StreamMJPEG::Id s_Id = 0;

StreamMJPEG::StreamMJPEG(const std::string& url, uint32_t textureId) :
	m_Error(Error::NoError),
	m_State(State::Initialising),
	m_Id(++s_Id),
	m_TextureId(textureId),
	m_pMultipartBlock(nullptr)
{
	m_pCurlMultiHandle = curl_multi_init();

	m_pCurlHandle = curl_easy_init();

	char pErrorBuffer[CURL_ERROR_SIZE];
	curl_easy_setopt(m_pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEFUNCTION, WriteResponseCallback);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(m_pCurlHandle, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_CONNECTTIMEOUT, 10L);

	curl_multi_add_handle(m_pCurlMultiHandle, m_pCurlHandle);
}

StreamMJPEG::~StreamMJPEG()
{
	curl_easy_cleanup(m_pCurlHandle);
}

void StreamMJPEG::Update()
{
	int running = INT_MAX;
	curl_multi_perform(m_pCurlMultiHandle, &running);

	if (running <= 1)
	{
		int a = 0;
	}

	if (m_pMultipartBlock)
	{
		CopyFrame(*m_pMultipartBlock);
	}
}

StreamMJPEG::State StreamMJPEG::GetState() const
{
	return m_State;
}

StreamMJPEG::Id StreamMJPEG::GetId() const
{
	return m_Id;
}

size_t StreamMJPEG::WriteHeaderCallback(void* pContents, size_t size, size_t nmemb, void* pUserData)
{
	size_t realSize = size * nmemb;
	StreamMJPEG* pStream = reinterpret_cast<StreamMJPEG*>(pUserData);
	std::vector<uint8_t>& data = pStream->m_HeaderBuffer;
	if (data.size() < realSize + 1)
	{
		data.resize(realSize + 1);
	}
	memcpy(&data[0], pContents, realSize);
	data[realSize] = '\0';

	std::string header(reinterpret_cast<const char*>(&data[0]));
	if (IsContentTypeHeader(header))
	{
		if (IsContentTypeMultipart(header))
		{
			if (ExtractMultipartBoundary(header, pStream->m_MultipartBoundary) == false)
			{
				pStream->m_Error = Error::UnknownBoundary;
			}
		}
		else
		{
			pStream->m_Error = Error::UnsupportedContentType;
		}
	}

	return realSize;
}

size_t StreamMJPEG::WriteResponseCallback(void* pContents, size_t size, size_t nmemb, void* pUserData)
{
	size_t realSize = size * nmemb;
	StreamMJPEG* pStream = reinterpret_cast<StreamMJPEG*>(pUserData);
	ByteArray& data = pStream->m_ResponseBuffer;
	size_t curDataSize = data.size();
	data.resize(curDataSize + realSize);
	memcpy(&data[curDataSize], pContents, realSize);
	ProcessMultipartContent(pStream);
	return realSize;
}

bool StreamMJPEG::IsContentTypeHeader(const std::string& header)
{
	const std::string contentType("Content-Type:");
	return (header.compare(0, contentType.size(), contentType.c_str()) == 0);
}

bool StreamMJPEG::IsContentTypeMultipart(const std::string& header)
{
	const std::string contentType("Content-Type: multipart/x-mixed-replace");
	return (header.compare(0, contentType.size(), contentType.c_str()) == 0);
}

bool StreamMJPEG::ExtractMultipartBoundary(const std::string& header, std::string& result)
{
	const std::string boundary("boundary=");
	size_t idx = header.find(boundary);
	if (idx == -1)
	{
		return false;
	}
	else
	{
		idx += boundary.size();
		size_t headerSize = header.size();
		size_t boundaryEnd = idx;
		while (boundaryEnd < headerSize)
		{
			char c = header[boundaryEnd];
			if (c == ' ' || c == '\r' || c == '\n')
			{
				break;
			}
			boundaryEnd++;
		}
		// The complete boundary is always in the format of "--boundary".
		result = std::string("--") + header.substr(idx, boundaryEnd - idx) + std::string("\r\n");
		return true;
	}
}

void StreamMJPEG::ProcessMultipartContent(StreamMJPEG* pStream)
{
	// If there's a boundary marker.
	size_t boundaryIdx = FindInStream(pStream, 0u, pStream->m_MultipartBoundary);
	if (boundaryIdx != -1)
	{
		bool emptyBlock = (boundaryIdx == 0);
		if (!emptyBlock)
		{
			if (pStream->m_pMultipartBlock != nullptr)
			{
				delete pStream->m_pMultipartBlock;
			}

			pStream->m_pMultipartBlock = new MultipartBlock(pStream->m_ResponseBuffer, boundaryIdx);
		}

		boundaryIdx = boundaryIdx + pStream->m_MultipartBoundary.size();
		size_t bytesToCopy = pStream->m_ResponseBuffer.size() - boundaryIdx;
		memmove(&pStream->m_ResponseBuffer[0], &pStream->m_ResponseBuffer[boundaryIdx], bytesToCopy * sizeof(uint8_t));
		pStream->m_ResponseBuffer.resize(bytesToCopy);
	}
}

size_t StreamMJPEG::FindInStream(StreamMJPEG* pStream, size_t offset, const std::string& toFind)
{
	size_t toFindSize = toFind.size();
	size_t toFindIdx = 0;
	size_t responseSize = pStream->m_ResponseBuffer.size();
	bool found = false;
	for (size_t i = offset; i < responseSize; ++i)
	{
		if (pStream->m_ResponseBuffer[i] == pStream->m_MultipartBoundary[toFindIdx])
		{
			toFindIdx++;
			if (toFindIdx == toFindSize)
			{
				return (i + 1) - toFindSize;
			}
		}
		else
		{
			toFindIdx = 0;
		}
	}

	return -1;
}

void StreamMJPEG::CopyFrame(const MultipartBlock& block)
{
	if (!block.IsValid()) // The block has not received the expected data or is missing headers.
	{
		return;
	}
	else if (block.GetType() != "image/jpeg") // It's a valid block, but not one we can process.
	{
		return;
	}

	const ByteArray& bytes = block.GetBytes();
	SDL_RWops* pOps = SDL_RWFromConstMem(&bytes[0], bytes.size());
	if (pOps != nullptr)
	{
		SDL_Surface* pSurface = IMG_LoadJPG_RW(pOps);
		if (pSurface == nullptr)
		{
			const char* pError = IMG_GetError();
			int a = 0;
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_TextureId);

			int bpp = pSurface->format->BytesPerPixel;
			if (bpp == 3 || bpp == 4)
			{
				int internalFormat = (bpp == 4) ? GL_RGBA : GL_RGB;
				int format = (bpp == 4) ? GL_RGBA : GL_RGB;

				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, pSurface->w, pSurface->h, 0, format, GL_UNSIGNED_BYTE, pSurface->pixels);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}

			SDL_FreeSurface(pSurface);
		}
	}
}