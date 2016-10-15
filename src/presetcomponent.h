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

		std::string mFileName;		//< Directory holding the preset
		std::string mPresetName;	//< Name of the preset
		PresetParts mParts;			//< All the associated preset parts
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

		// Loading
		void									loadPresets();

	private:
		// Preset directory
		ofDirectory mPresetDir;
		std::vector<std::unique_ptr<Preset>>	mPresets;

		// Preset Name
		NSLOT(mPresetChanged, const int&, presetChanged)
		void presetChanged(const int& idx);
	};
}

RTTI_DECLARE(nap::PresetComponent)
