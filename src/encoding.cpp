#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#include <SDL.h>

#include "encoding.h"

namespace Encoding
{

// Convert a wide Unicode string to an UTF8 string
std::string UTF8Encode(const std::wstring& wstr)
{
#ifdef _WIN32
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
#else
	SDL_assert(false); // Not implemented.
	return std::string();
#endif
}

// Convert an UTF8 string to a wide Unicode String
std::wstring UTF8Decode(const std::string& str)
{
#ifdef _WIN32
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
#else
	SDL_assert(false); // Not implemented.
	return std::wstring();
#endif
}

std::string URLEncode(const std::string& str)
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::value_type c : str) 
	{
		// Keep alphanumeric and other accepted characters intact.
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') 
		{
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded.
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	return escaped.str();
}

} // namespace Encoding
