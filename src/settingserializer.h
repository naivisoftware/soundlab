#pragma once

#include <string>
#include <gui.h>

/**
@brief Simple serializer used to save and load settings
**/
class SettingSerializer
{
public:
	SettingSerializer() {};
	virtual ~SettingSerializer() {};

	// Loads / Saves all settings to disk
	void loadSettings(const std::string& dir, const Gui& gui);
	void saveSettings(const std::string& dir, const std::string& name, const Gui& gui);
};