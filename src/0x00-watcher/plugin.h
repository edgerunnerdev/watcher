#include "json.h"

#ifdef _WIN32
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API
#endif //_WIN32

class Plugin
{
public:
	virtual bool Initialise() = 0;
	virtual void OnMessage( const nlohmann::json& message ) = 0;
};

#define DECLARE_PLUGIN( PluginClass, MajorVersion, MinorVersion, PatchVersion ) \
	extern "C" \
	{ \
		PLUGIN_API Plugin* GetPlugin() \
		{ \
			static PluginClass sSingleton; \
			return &sSingleton; \
		} \
		PLUGIN_API const char* GetPluginName() \
		{ \
			return #PluginClass; \
		} \
		PLUGIN_API void GetPluginVersion( int& major, int& minor, int &patch ) \
		{ \
			major = MajorVersion; \
			minor = MinorVersion; \
			patch = PatchVersion; \
		} \
	}
		