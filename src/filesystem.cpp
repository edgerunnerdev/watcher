#include <string>
#include "filesystem.h"

#if _MSC_VER >= 1910
#define HAS_EXPERIMENTAL_FILESYSTEM (1)
#else
#define HAS_EXPERIMENTAL_FILESYSTEM (0)
#endif

#if HAS_EXPERIMENTAL_FILESYSTEM
#include <filesystem>
#else

#endif

bool Filesystem::FileExists( const std::string& filename )
{
#if HAS_EXPERIMENTAL_FILESYSTEM
	return std::experimental::filesystem::exists( filename );
#else

#endif
}

bool Filesystem::CreateDirectories( const std::string& path )
{
#if HAS_EXPERIMENTAL_FILESYSTEM
	return std::experimental::filesystem::create_directories( path );
#else

#endif
}
