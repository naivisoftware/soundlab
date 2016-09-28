#include <settings.h>
#include <ofParameterGroup.h>
#include <nap/logger.h>


// Constructor
ConfigFile::ConfigFile(const std::string& file) : mFile(file)
{}


// Returns settings associated with file 
ofxXmlSettings& ConfigFile::getSettings()
{
	if (mIsLoaded)
		return mSettings;

	ofFile file(mFile);
	if (!file.exists())
	{
		nap::Logger::info("settings file does not exist: %s, creating new one", mFile.c_str());
		mIsLoaded = createFile();
	}
	else
	{
		mIsLoaded = mSettings.loadFile(mFile);
		if (!mIsLoaded)
		{
			nap::Logger::warn("unable to load file: %s", mFile.c_str());
		}
	}
	return mSettings;
}


// Creates a new file
bool ConfigFile::createFile() const
{
	// Try to save
	if (!mSettings.saveFile(mFile))
	{
		nap::Logger::warn("unable to save settings file: %s", mFile.c_str());
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////

// App settings
static const std::string sAppSettingsFile("app_settings.xml");
ConfigFile& gGetAppSettings()
{
	static ConfigFile app_file(sAppSettingsFile);
	return app_file;
}


