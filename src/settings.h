#pragma once

#include <ofxXmlSettings.h>

// Config file wrapper
class ConfigFile
{
public:
	ConfigFile(const std::string& file);

	// Getters
	ofxXmlSettings&			getSettings();
	std::string				getFileName() const { return mFile; }
	template <typename T>
	T						getSetting(const std::string& name, const T& defaultValue);

private:
	std::string				mFile;
	mutable bool			mIsLoaded = false;
	mutable ofxXmlSettings	mSettings;

	// Creates settings file if it doesn't exist
	bool createFile() const ;
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
T ConfigFile::getSetting(const std::string& name, const T& defaultValue)
{
	ofxXmlSettings& settings = getSettings();
	return settings.getValue(name, defaultValue);
}

//////////////////////////////////////////////////////////////////////////

ConfigFile& gGetAppSettings();

template <typename T>
T gGetAppSetting(const std::string& name, const T& defaultValue)
{
	return gGetAppSettings().getSetting<T>(name, defaultValue);
}