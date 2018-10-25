#pragma once

#include "json.h"
using json = nlohmann::json;

#ifdef _WIN32
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API
#endif //_WIN32

struct ImGuiContext;
using PluginMessageCallback = void (*)( const json& message );

class Plugin
{
public:
	virtual bool Initialise( PluginMessageCallback pMessageCallback ) = 0;
	virtual void OnMessageReceived( const json& message ) = 0;
	virtual void DrawUI( ImGuiContext* pContext ) = 0;

	virtual std::string GetName() const = 0;
	virtual void GetVersion( int& majorVersion, int& minorVersion, int& patchVersion ) const = 0;
};
		
#define DECLARE_PLUGIN( PluginClass, MajorVersion, MinorVersion, PatchVersion ) \
	public: \
		virtual std::string GetName() const override { return #PluginClass; } \
		virtual void GetVersion( int& majorVersion, int& minorVersion, int& patchVersion ) const override \
			{ \
				majorVersion = MajorVersion; \
				minorVersion = MinorVersion; \
				patchVersion = PatchVersion; \
			} \
	private:

#define IMPLEMENT_PLUGIN( PluginClass ) \
	extern "C" \
	{ \
		PLUGIN_API Plugin* GetPlugin() \
		{ \
			static PluginClass sSingleton; \
			return &sSingleton; \
		} \
	}