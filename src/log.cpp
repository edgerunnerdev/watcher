///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#ifdef _WIN32
#include "windows.h"
#endif

#include "log.h"

namespace Watcher
{

//////////////////////////////////////////////////////////////////////////
// Log
//////////////////////////////////////////////////////////////////////////

std::mutex Log::m_Mutex;
Log::LogTargetList Log::m_Targets;
std::array< char, sLogBufferSize > Log::m_Buffer;
std::array< char, sLogBufferSize > Log::m_VABuffer;

void Log::AddLogTarget( LogTargetSharedPtr pLogTarget )
{
	std::lock_guard< std::mutex > lock( m_Mutex );
	m_Targets.push_back( pLogTarget );
}

void Log::RemoveLogTarget( LogTargetSharedPtr pLogTarget )
{
	std::lock_guard< std::mutex > lock( m_Mutex );
	m_Targets.remove( pLogTarget );
}

// Internal logging function. Should be called by one of the public functions (LogInfo / LogWarning / LogError).
// Assumes that m_Mutex is locked at this stage.
void Log::LogInternal( const std::string& text, LogLevel level )
{
	std::string prefix;
	if ( level == LogLevel::Info ) prefix = "INFO: ";
	else if ( level == LogLevel::Warning ) prefix = "WARNING: ";
	else if ( level == LogLevel::Error ) prefix = "ERROR: ";

    snprintf( m_Buffer.data(), m_Buffer.size(), "%s%s\n", prefix.c_str(), text.c_str() );

    for ( auto& pTarget : m_Targets )
    {
		pTarget->Log( m_Buffer.data(), level );
    }
}

void Log::Info( const char* format, ... )
{
	std::lock_guard< std::mutex > lock( m_Mutex );

    va_list args;
    va_start( args, format );
	vsnprintf( m_VABuffer.data(), m_VABuffer.size(), format, args );
    LogInternal( m_VABuffer.data(), LogLevel::Info );
    va_end( args );
}

void Log::Warning( const char* format, ... )
{
	std::lock_guard< std::mutex > lock( m_Mutex );

    va_list args;
    va_start( args, format );
    vsnprintf( m_VABuffer.data(), m_VABuffer.size(), format, args );
    LogInternal( m_VABuffer.data(), LogLevel::Warning );
    va_end( args );
}

void Log::Error( const char* format, ... )
{
	std::lock_guard< std::mutex > lock( m_Mutex );

    va_list args;
    va_start( args, format );
    vsnprintf( m_VABuffer.data(), m_VABuffer.size(), format, args );
    LogInternal( m_VABuffer.data(), LogLevel::Error );
    va_end( args );

#ifdef _WIN32
	__debugbreak();
#else
    asm("int $3");
#endif
}


//////////////////////////////////////////////////////////////////////////
// FileLogger
//////////////////////////////////////////////////////////////////////////

FileLogger::FileLogger( const char* pFilename )
{
    m_File.open( pFilename, std::fstream::out | std::fstream::trunc );
}

FileLogger::~FileLogger()
{
    if ( m_File.is_open() )
    {
        m_File.close();
    }
}

void FileLogger::Log( const std::string& text, LogLevel level )
{
    if ( !m_File.is_open() )
    {
        return;
    }

    m_File.write( text.c_str(), text.size() );
    m_File.flush();
}


//////////////////////////////////////////////////////////////////////////
// MessageBoxLogger
//////////////////////////////////////////////////////////////////////////

//void MessageBoxLogger::Log( char* pText, LogMessageType type )
//{
//    if ( type == LOG_WARNING )
//    {
//        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_WARNING, "Warning", pText, nullptr );
//    }
//    else if ( type == LOG_ERROR )
//    {
//        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error", pText, nullptr );
//    }
//}


//////////////////////////////////////////////////////////////////////////
// VisualStudioLogger
//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void VisualStudioLogger::Log( const std::string& text, LogLevel level )
{
    OutputDebugStringA( text.c_str() );
}
#endif

} // namespace Watcher
