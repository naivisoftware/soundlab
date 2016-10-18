#pragma once

// Nap Includes
#include <nap/component.h>
#include <nap/attribute.h>
#include <rtti/rtti.h>
#include <ofXml.h>

// Of Includes
#include <ofFileUtils.h>
#include <ofBaseTypes.h>
#include <nap/coremodule.h>
#include <napofupdatecomponent.h>
#include <nap/componentdependency.h>

namespace nap
{
	/**
	@brief Represents a part of a preset
	**/
	struct PresetPart
	{
		PresetPart(const std::string& file);

		ofXml mSerializer;		//< Holds all the settings
		std::string	mPartName;	//< Holds the name of the file
		std::string mFileName;	//< Holds the file that is loaded using the serializer
		bool mLoaded = false;
	};


	/**
	@brief Represents a preset
	**/
	struct Preset
	{
		using PresetParts = std::vector<std::unique_ptr<PresetPart>>;

		Preset(const std::string& directory);

		std::string mFileName;				//< Directory holding the preset
		std::string mPresetName;			//< Name of the preset
		PresetParts mParts;					//< All the associated preset parts
		float		mDuration = -1.0f;		//< Duration of the preset
	};


	/**
	@brief Manages all the presets
	Caches them and allows for cycling through them
	Handling changes needs to be done elsewhere (app)
	**/
	class PresetComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)
	public:
		PresetComponent();

		// Current Selection
		NumericAttribute<int>					index									{ this, "Preset", 0, 0, 1 };
		nap::Attribute<std::string>				presetName								{ this, "Name" };

		// Setters / Getters
		void									setDirectory(const ofDirectory& dir)	{ mPresetDir = dir; loadPresets(); }
		const ofDirectory&						getDirectory() const					{ return mPresetDir; }
		Preset*									getPreset(int index);
		void									setPreset(std::string& name);
		int										getPresetCount()						{ return mPresets.size(); }
		Preset*									getCurrentPreset();

		// Loading
		void									loadPresets();

	private:
		// Preset directory
		ofDirectory mPresetDir;
		std::vector<std::unique_ptr<Preset>>	mPresets;
		nap::Preset*							mCurrentPreset = nullptr;

		// Populate tag
		void									populateTags(Preset& preset);

		// Preset Name
		NSLOT(mPresetChanged, const int&, presetChanged)
		void presetChanged(const int& idx);
	};


	/**
	@brief Switches between presets based on a speed and random offset
	**/
	class PresetSwitchComponent : public OFUpdatableComponent 
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)
	public:
		// Default constructor
		PresetSwitchComponent();

		// Update call
		virtual void onUpdate() override;

		// Settings
		NumericAttribute<float> time  = { this, "Time", 1.0f, 0.25f, 60.0f };				//< Time it takes to switch to another preset
		NumericAttribute<float> offset = { this, "RandomOffset", 0.0f, 0.0f, 1.0f };		//< Amount of random switch offset
		
		void reset();

	private:
		ComponentDependency<PresetComponent> mPresetComponent	{ this };

		// Time
		float mTargetTime = 0.0f;
		float mStartTime  = 0.0f;

		// Reset Slot
		NSLOT(mUpdateChanged, const bool&,  updateChanged)
		NSLOT(mSpeedChanged,  const float&, speedChanged)
		NSLOT(mOffsetChanged, const float&, offsetChanged)
		
		void updateChanged(const bool& value);
		void speedChanged(const float& value)	{ updateTargetTime(); }
		void offsetChanged(const float& value)	{ updateTargetTime(); }


		
		// Updates to a new target time
		void updateTargetTime();
	};
}

RTTI_DECLARE(nap::PresetComponent)
RTTI_DECLARE(nap::PresetSwitchComponent)
