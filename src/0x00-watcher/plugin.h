#include "json.h"

#ifdef _WIN32
#ifdef _USRDLL
#define PLUGINAPI __declspec(dllexport)
#else
#define PLUGINAPI __declspec(dllimport)
#endif // _USRDLL
#endif //_WIN32

int PLUGINAPI GetPluginVersion();

class Plugin
{
public:
	virtual bool Initialise() = 0;
	virtual void OnMessage( const nlohmann::json& message ) = 0;
};
