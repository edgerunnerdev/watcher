#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <list>
#include <stdarg.h>
#include <string>
#include <vector>

class ILogTarget;

//////////////////////////////////////////////////////////////////////////
// Log
// Contains any number of ILogTargets, which are responsible for actually 
// logging the message in various ways. 
// This class is thread safe.
//////////////////////////////////////////////////////////////////////////

using LogTargetSharedPtr = std::shared_ptr< ILogTarget >;
static const size_t sLogBufferSize = 20480u;

enum class LogLevel
{
	Info,
	Warning,
	Error
};

class Log
{
public:
    static void Info( const char* pFormat, ... );
    static void Warning( const char* pFormat, ... );
    static void Error( const char* pFormat, ... );

    static void AddLogTarget( LogTargetSharedPtr pLogTarget );
    static void RemoveLogTarget( LogTargetSharedPtr pLogTarget );

private:
	using LogTargetList = std::list< LogTargetSharedPtr >;

    static void LogInternal( const std::string& text, LogLevel level );

    static std::mutex m_Mutex;
    static LogTargetList m_Targets;
    static std::array< char, sLogBufferSize > m_Buffer;
    static std::array< char, sLogBufferSize > m_VABuffer;
};


//////////////////////////////////////////////////////////////////////////
// ILogTarget
// Any ILogTarget must implement Log().
//////////////////////////////////////////////////////////////////////////

class ILogTarget
{
public:
	virtual ~ILogTarget() {}
    virtual void Log( const std::string& text, LogLevel level ) = 0;
};


//////////////////////////////////////////////////////////////////////////
// FileLogger
// Dumps the logging into file given in "filename". It is flushed
// after every entry.
//////////////////////////////////////////////////////////////////////////

class FileLogger : public ILogTarget
{
public:
    FileLogger( const char* pFilename );
    virtual ~FileLogger() override;
    virtual void Log( const std::string& text, LogLevel type ) override;

private:
    std::ofstream m_File;
};


//////////////////////////////////////////////////////////////////////////
// MessageBoxLogger
// Creates a message box whenever the log message is above LogLevel::Info.
//////////////////////////////////////////////////////////////////////////

class MessageBoxLogger : public ILogTarget
{
public:
	virtual void Log( const std::string& text, LogLevel type ) override;
};


//////////////////////////////////////////////////////////////////////////
// VisualStudioLogger
// Windows only - all the output from the logger goes to the
// Visual Studio output window.
//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
class VisualStudioLogger : public ILogTarget
{
public:
	virtual void Log( const std::string& text, LogLevel type ) override;
};
#endif
