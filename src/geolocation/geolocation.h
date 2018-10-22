#include "..\0x00-watcher\plugin.h"

class Geolocation : public Plugin
{
public:
	bool Initialise() override;
	void OnMessage(const nlohmann::json& message) override;
};