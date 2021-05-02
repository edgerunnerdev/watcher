#pragma once

#include <string>

class Filesystem
{
public:
	static bool FileExists( const std::string& filename );
	static bool CreateDirectories( const std::string& path );
};