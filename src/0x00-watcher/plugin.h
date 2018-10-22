#include "json.h"

#ifdef _WIN32
#ifdef _USRDLL
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif // _USRDLL
#endif //_WIN32

class Plugin
{
public:
	virtual bool Initialise() = 0;
	virtual void OnMessage( const nlohmann::json& message ) = 0;
};

extern "C"
{
	PLUGIN_API Plugin* GetPlugin();
}

#define DECLARE_PLUGIN( PluginClass ) \
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
	}
		