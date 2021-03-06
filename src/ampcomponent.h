#pragma once

#include <nap/component.h>
#include <nap/coremodule.h>
#include <napofupdatecomponent.h>
#include <ofVec2f.h>
#include <napofattributes.h>
#include <napofsplinecomponent.h>

namespace lib
{
	namespace audio
	{
		class AudioService;
	}
}

namespace nap
{
	/**
	@brief Modifies intensity of spline based on rms amplitude
	**/
	class AmpIntensityComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		AmpIntensityComponent();
		
		// Parameters
		Attribute<bool>				invert		{ this, "Invert", false };
		NumericAttribute<float>		influence	{ this, "Influence", 1.0f, 0.0f, 1.0f };
		NumericAttribute<float>		scale		{ this, "FinalScale", 1.0f, 0.0f, 1.0f };
		NumericAttribute<ofVec2f>	range		{ this, "Range", {0.025f, 0.3f}, {0.0f, 0.001f}, {1.0f, 1.0f} };
		NumericAttribute<float>		damping		{ this, "Damping", 0.05f, 0.0f, 1.0f };

		// Link
		Link colorComponent	{ *this };

		// Update call
		virtual void onUpdate() override;

	private:
		// Audio service
		lib::audio::AudioService* mAudioService = nullptr;

		NSLOT(mAdded, const Object&, onAdded)
		void onAdded(const Object& obj);

		// Smooth Damp
		float mCurrentAmp = 0.0f;
		float mCurrentVel = 0.0f;
	};


	/**
	@brief Modifies scale of spline based on rms amplitude
	**/
	class AmpScaleComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		AmpScaleComponent();

		// Parameters
		Attribute<bool>				invert		{ this, "Invert", false };
		NumericAttribute<float>		influence	{ this, "Influence", 1.0f, 0.0f, 1.0f };
		NumericAttribute<ofVec2f>	range		{ this, "Range",{ 0.025f, 0.3f },{ 0.0f, 0.001f },{ 1.0f, 1.0f } };
		NumericAttribute<float>		start		{ this, "DefaultValue", 1.0f, 0.0f, 1.0f };
		NumericAttribute<float>		scale		{ this, "Scale", 1.0f, 0.0f, 2.5f };
		NumericAttribute<float>		damping		{ this, "Damping", 0.05f, 0.0f, 1.0f };
		NumericAttribute<int>		mode		{ this, "BlendMode", 0, 0, 2 };

		// Link
		Link transformLink{ *this };

		// Update call
		virtual void onUpdate() override;

	private:
		// Audio service
		lib::audio::AudioService* mAudioService = nullptr;

		NSLOT(mAdded, const Object&, onAdded)
			void onAdded(const Object& obj);

		// Smooth Damp
		float mCurrentAmp = 0.0f;
		float mCurrentVel = 0.0f;
	};


	/**
	@brief Modifies scale of spline based on rms amplitude
	**/
	class AmpRotateComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		AmpRotateComponent();

		// Parameters
		Attribute<bool>				invert			{ this, "Invert", false };
		NumericAttribute<float>		influence		{ this, "Influence", 1.0f, 0.0f, 1.0f };
		NumericAttribute<ofVec2f>	range			{ this, "Range",{ 0.025f, 0.3f },{ 0.0f, 0.001f },{ 1.0f, 1.0f } };
		NumericAttribute<float>		start			{ this, "DefaultValue", 0.0f, 0.0f, 100.0f };
		NumericAttribute<float>		scale			{ this, "Scale", 1.0f, 0.0f, 1080.0f };
		NumericAttribute<float>		damping			{ this, "Damping", 0.05f, 0.0f, 1.0f };

		// Link
		Link rotateLink{ *this };

		// Update call
		virtual void onUpdate() override;

	private:
		// Audio service
		lib::audio::AudioService* mAudioService = nullptr;

		NSLOT(mAdded, const Object&, onAdded)
			void onAdded(const Object& obj);

		// Smooth Damp
		float mCurrentAmp = 0.0f;
		float mCurrentVel = 0.0f;
	};


	/**
	@brief Modifies amplitude of lfo
	**/
	class AmpLFOComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		AmpLFOComponent();

		// Parameters
		Attribute<bool>				invert{ this, "Invert", false };
		NumericAttribute<float>		influence{ this, "Influence", 1.0f, 0.0f, 1.0f };
		NumericAttribute<ofVec2f>	range{ this, "Range",{ 0.025f, 0.3f },{ 0.0f, 0.001f },{ 1.0f, 1.0f } };
		NumericAttribute<float>		start{ this, "DefaultValue", 0.0f, 0.0f, 10.0f };
		NumericAttribute<float>		scale{ this, "Scale", 1.0f, 0.0f, 10.0f };
		NumericAttribute<float>		damping{ this, "Damping", 0.05f, 0.0f, 1.0f };

		// Link
		Link lfoLink{ *this };

		// Update call
		virtual void onUpdate() override;

	private:
		// Audio service
		lib::audio::AudioService* mAudioService = nullptr;

		NSLOT(mAdded, const Object&, onAdded)
			void onAdded(const Object& obj);

		// Smooth Damp
		float mCurrentAmp = 0.0f;
		float mCurrentVel = 0.0f;
	};
}

RTTI_DECLARE(nap::AmpIntensityComponent)
RTTI_DECLARE(nap::AmpScaleComponent)
RTTI_DECLARE(nap::AmpRotateComponent)
RTTI_DECLARE(nap::AmpLFOComponent)