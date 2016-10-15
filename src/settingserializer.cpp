#include <settingserializer.h>
#include <nap/stringutils.h>

/**
@brief Loads all settings from disk and applies them
**/
void SettingSerializer::loadSettings(const std::string& dir, const Gui& gui)
{
	// Get directory to load
	ofDirectory settings_dir(dir);
	if (!settings_dir.exists())
	{
		nap::Logger::warn("unable to load preset, dir does not exist: %s", settings_dir.getAbsolutePath().c_str());
		return;
	}

	// Apply all the files
	for (auto& f : settings_dir.getFiles())
	{
		if (f.getExtension() != "xml")
		{
			nap::Logger::warn("skipping file: %s, not an xml", f.getFileName().c_str());
			continue;
		}

		// Get file name
		std::string file_name = f.getBaseName();

		// Find it in the available guis
		auto result = std::find_if(gui.getGuis().begin(), gui.getGuis().end(),[&](const auto& gui)
		{
			return gui->getName() == file_name;
		});

		// Make sure we found something
		if (result == gui.getGuis().end())
		{
			nap::Logger::warn("unable to apply settings for file: %s", f.getFileName().c_str());
			continue;
		}

		// Load settings
		(*result)->loadFromFile(f.getAbsolutePath());
	}
}



/**
@brief Saves all settings to disk
**/
void SettingSerializer::saveSettings(const std::string& dir, const std::string& name, const Gui& gui)
{
	// Create save dir
	ofDirectory save_dir(dir + "/" + name);
	if (save_dir.exists())
	{
		nap::Logger::warn("save directory already exists: %s", save_dir.getAbsolutePath().c_str());
		nap::Logger::warn("files will be replaced");
	}
	else
	{
		if(!save_dir.create(true))
		{
			nap::Logger::warn("unable to create directory: %s", save_dir.getAbsolutePath().c_str());
			return;
		}
	}

	// Save all guis
	for (const auto& gui : gui.getGuis())
	{
		std::string gui_name = gui->getName();
		gui->saveToFile(save_dir.getAbsolutePath() + "/" + gui_name + ".xml");
	}
}

