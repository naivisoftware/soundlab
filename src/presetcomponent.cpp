#include <presetcomponent.h>
#include <nap/logger.h>
#include <Utils/nofUtils.h>

namespace nap
{
	/**
	@brief Preset part constructor
	**/
	PresetPart::PresetPart(const std::string& file) : mFileName(file)
	{
		ofFile part_file(file);
		if (!(part_file.exists()))
		{
			nap::Logger::warn("unable to construct preset part, file does not exist: %s", part_file.getAbsolutePath().c_str());
			return;
		}

		// Load the file
		if (!mSerializer.load(mFileName))
		{
			nap::Logger::warn("unable to deserialize preset part from file: %s", file.c_str());
			return;
		}

		// Store part name
		mPartName = part_file.getBaseName();
		mLoaded = true;
	}


	/**
	@brief Preset constructor
	**/
	Preset::Preset(const std::string& directory)
	{
		ofFile preset_location(directory);
		if (!preset_location.isDirectory())
		{
			nap::Logger::warn("invalid preset directory: %s", directory.c_str());
			return;
		}

		// Set preset name
		mPresetName = preset_location.getBaseName();
		
		// Wrap in dir
		ofDirectory preset_dir(directory);
		preset_dir.listDir();

		for (auto& file : preset_dir.getFiles())
		{
			if (file.getExtension() != "xml")
			{
				nap::Logger::warn("skipping file: %s, not an xml", file.getFileName().c_str());
				continue;
			}

			// Add as a part
			mParts.emplace_back(std::make_unique<PresetPart>(file.getAbsolutePath()));
		}
	}


	/**
	@brief Constructor
	**/
	PresetComponent::PresetComponent()
	{
		index.valueChangedSignal.connect(mPresetChanged);
	}


	/**
	@brief Returns a preset based on index
	**/
	Preset* PresetComponent::getPreset(int index)
	{
		if (index >= mPresets.size() || index < 0)
		{
			nap::Logger::warn("preset index out of bounds!");
			return nullptr;
		}
		return mPresets[index].get();
	}

	/**
	@brief Loads all the presets
	**/
	void PresetComponent::loadPresets()
	{
		if (!mPresetDir.exists())
		{
			nap::Logger::warn("preset directory doesn't exist: %s", mPresetDir.getOriginalDirectory().c_str());
			return;
		}

		// Clear existing presets
		mPresets.clear();

		// Walk over all the internal directories
		mPresetDir.listDir();
		for (auto& file : mPresetDir.getFiles())
		{
			if(!file.isDirectory())
				continue;

			// Add an entry
			mPresets.emplace_back(std::make_unique<Preset>(file.getAbsolutePath()));
			nap::Logger::info("loading preset: %s", file.getBaseName().c_str());
		}

		index.setRange(0, gMax<int>(mPresets.size() - 1, 0));
	}


	/**
	@brief Updates the current preset name
	**/
	void PresetComponent::presetChanged(const int& idx)
	{
		Preset* preset = getPreset(idx);
		assert(preset != nullptr);
		presetName.setValue(preset->mPresetName);
	}

}


RTTI_DEFINE(nap::PresetComponent)