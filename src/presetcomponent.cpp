#include <presetcomponent.h>
#include <nap/logger.h>
#include <Utils/nofUtils.h>
#include <settings.h>
#include <nap/stringutils.h>

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
		mFileName = preset_location.getAbsolutePath();

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
			std::unique_ptr<PresetPart> part_ptr = std::make_unique<PresetPart>(file.getAbsolutePath());
			mParts.emplace_back(std::move(part_ptr));
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


	void PresetComponent::setPreset(std::string& name)
	{
		int idx(0);
		for (auto& preset : mPresets)
		{
			if (preset->mPresetName == name)
			{
				index.setValue(idx);
				return;
			}
			idx++;
		}
		
		nap::Logger::warn(*this, "unable to find preset with name: %s", name.c_str());
	}



	/**
	@brief Returns the current preset
	**/
	Preset* PresetComponent::getCurrentPreset()
	{
		return mCurrentPreset;
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

		// Get current preset name to match later on
		std::string current_preset_name = mCurrentPreset != nullptr ? mCurrentPreset->mPresetName : "";

		// Clear existing presets
		mPresets.clear();

		// Make sure we don't have a current preset
		mCurrentPreset = nullptr;

		// Walk over all the internal directories
		mPresetDir.listDir();
		int preset_idx(0), current_preset_idx(-1);
			for (auto& file : mPresetDir.getFiles())
		{
			if(!file.isDirectory())
				continue;

			std::unique_ptr<Preset> new_preset = std::make_unique<Preset>(file.getAbsolutePath());
			if (new_preset->mPresetName == current_preset_name)
			{
				mCurrentPreset = new_preset.get();
				current_preset_idx = preset_idx;
			}

			// populate preset part name
			populateTags(*new_preset);

			// Add an entry
			mPresets.emplace_back(std::move(new_preset));
			preset_idx++;
		}

		// Update range
		index.setRange(0, gMax<int>(mPresets.size() - 1, 0));

		// Set new preset value based on string match
		if (mCurrentPreset != nullptr)
		{
			index.setValue(current_preset_idx);
		}
	}


	/**
	@brief Populates tag values in preset
	**/
	void PresetComponent::populateTags(Preset& preset)
	{
		// Find right part
		std::string tag_file_name = gGetAppSetting<std::string>("TagFile", "spline");
		const PresetPart* preset_part(nullptr);
		for (const auto& part : preset.mParts)
		{
			if (part->mPartName == tag_file_name)
			{
				preset_part = part.get();
				break;
			}
		}

		// Make sure we have the part
		if (preset_part == nullptr)
		{
			nap::Logger::warn(*this, "unable to fetch file: %s that contains tag information", tag_file_name.c_str());
			return;
		}

		// Get root
		const Poco::XML::Element* current_element = preset_part->mSerializer.getPocoElement();
		if (current_element == nullptr)
		{
			nap::Logger::warn(*this, "tag file has no root element!");
			return;
		}

		std::string tag_path = gGetAppSetting<std::string>("TagPath", "Tag");
		std::vector<std::string> out_children;
		gSplitString(tag_path, '/', out_children);

		// Find child
		const Poco::XML::Element* tag_element(nullptr);
		for (const auto& child : out_children)
		{
			current_element = current_element->getChildElement(child);
		}

		// Make sure we have one
		if (current_element == nullptr)
		{
			nap::Logger::warn("Unable to find tag element with path: %s", tag_path.c_str());
			return;
		}

		// Fetch values
		const Poco::XML::Element* time_element = current_element->getChildElement("PresetTime");
		if (time_element == nullptr)
		{
			nap::Logger::warn(*this, "no preset time element found: %s", current_element->nodeName().c_str());
			return;
		}

		float preset_time = std::atof(time_element->innerText().c_str());
		preset.mDuration = preset_time;
	}


	/**
	@brief Updates the current preset name
	**/
	void PresetComponent::presetChanged(const int& idx)
	{
		Preset* preset = getPreset(idx);
		assert(preset != nullptr);
		mCurrentPreset = preset;
		presetName.setValue(mCurrentPreset->mPresetName);
	}


	/**
	@brief Preset switch component constructor
	**/
	PresetSwitchComponent::PresetSwitchComponent()
	{
		mEnableUpdates.setValue(false);

		// When update is enabled / disabled -> reset time and target time
		mEnableUpdates.valueChangedSignal.connect(mUpdateChanged);

		// Connect time and offset
		time.valueChangedSignal.connect(mSpeedChanged);
		offset.valueChangedSignal.connect(mOffsetChanged);
		scale.valueChangedSignal.connect(mScaleChanged);
		progress.setClamped(true);

		reset();
	}


	/**
	@brief Switches preset when time has elapsed
	**/
	void PresetSwitchComponent::onUpdate()
	{
		// Get
		float current_time = ofGetElapsedTimef();

		// Check time difference
		float diff_time = current_time - mStartTime;
		progress.setValue(diff_time / mTargetTime);

		if (diff_time < mTargetTime)
			return;

		// Time passed, select new preset
		PresetComponent* preset_comp = mPresetComponent.get();
		if (preset_comp == nullptr)
		{
			nap::Logger::warn(*this, "can't switch preset, no preset component found");
			return;
		}

		// Don't do anything when there are no presets
		if (preset_comp->getPresetCount() == 0)
			return;

		// Select a new preset when time has passed;
		selectNewPreset(*preset_comp);
	}
	
	/**
	@brief Reset all time codes
	**/
	void PresetSwitchComponent::reset()
	{
		mStartTime = ofGetElapsedTimef();
		updateTargetTime();
	}

	/**
	@brief When enable update changes, reset target time
	**/
	void PresetSwitchComponent::updateChanged(const bool& value)
	{
		reset();
	}

	/*
	@brief Updates the speed, only when override is turned off
	*/
	void PresetSwitchComponent::speedChanged(const float& value)
	{
		if (fromPreset.getValue())
			return;
		updateTargetTime();
	}


	// Selects a new preset
	// Also updates the time if override is turned off (based on preset value)
	void PresetSwitchComponent::selectNewPreset(PresetComponent& presetComp)
	{
		int new_preset_idx = presetComp.index;
		if (presetComp.getPresetCount() > 1)
		{
			while (new_preset_idx == presetComp.index)
			{
				new_preset_idx = gMin<int>((int)ofRandom(presetComp.getPresetCount()), presetComp.getPresetCount() - 1);
			}
		}

		// If we're picking the preset's value, do so
		if (fromPreset.getValue())
		{
			// Set new time value
			Preset* new_preset = presetComp.getPreset(new_preset_idx);
			time.setValue(new_preset->mDuration);
		}

		// Reset
		reset();

		// Set new preset index
		presetComp.index.setValue(new_preset_idx);
	}

	/**
	@brief updates the target time
	**/
	void PresetSwitchComponent::updateTargetTime()
	{
		float min_time = time.getValue() - (time.getValue() * offset.getValue());
		float max_time = time.getValue() + (time.getValue() * offset.getValue());
		mTargetTime = ofRandom(min_time, max_time) * scale.getValue();

		nap::Logger::info("new preset target time: %f",  mTargetTime);
	}

}


RTTI_DEFINE(nap::PresetComponent)
RTTI_DEFINE(nap::PresetSwitchComponent)