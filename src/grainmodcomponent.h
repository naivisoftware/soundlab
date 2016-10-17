#pragma once

#include <napofupdatecomponent.h>
#include <nap/coremodule.h>
#include <napofattributes.h>
#include <rtti/rtti.h>
#include <audio.h>
#include <ofTypes.h>
#include <napofsplinecomponent.h>

namespace nap
{
	class GrainModComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		GrainModComponent();

		// Update override
		virtual void				onUpdate() override;

		// To be implemented by derived classes
		virtual void				updateParameter() = 0;
		virtual void				triggered(lib::TimeValue& time, const lib::audio::GrainParameters& params, float timeDiff) = 0;

		// Parameters
		NumericAttribute<float> acceptance =	{ this, "Acceptance", 1.0f, 0.0f, 1.0f };		//< Acceptance based on random value		
		NumericAttribute<float>	cutoff =		{ this, "TimeCutoff", 0.0f, 0.0f, 2.0f };			//< Don't except signals under a certain time threshold

		// Register this component to listen to a grain signal
		void						registerGrainSignal(AudioComposition& inComposition);

	private:

		// Trigger related values
		ofMutex						mMutex;
		lib::TimeValue				mTriggerTime;
		lib::audio::GrainParameters mTriggerParameters;
		bool						mTriggered = false;

		// Time
		float						mPreviousTriggerTime = 0.0f;

		// Slot used when receiving trigger signal
		void grainTriggered(lib::TimeValue& time, const lib::audio::GrainParameters& params);
		nap::Slot<lib::TimeValue, const lib::audio::GrainParameters&> mGrainTriggered = { [&](lib::TimeValue time, const lib::audio::GrainParameters& params)
		{
			mMutex.lock();
			mTriggered = true;
			mTriggerTime = time;
			mTriggerParameters = params;
			mMutex.unlock();
		} };
	};


	class GrainColorModComponent : public GrainModComponent
	{
		RTTI_ENABLE_DERIVED_FROM(GrainModComponent)
	public:
		GrainColorModComponent();

		virtual void updateParameter() override;
		virtual void triggered(lib::TimeValue& time, const lib::audio::GrainParameters& params, float timeDiff) override;

		// Damp attribute
		NumericAttribute<float>		damp =				{ this, "Damp",  0.025f, 0.0f, 1.0f };
		NumericAttribute<ofVec2f>	frequency_range =	{ this, "FrequencyRange", {1.0f, 10.0f} , {1.0f, 1.0f}, {25.0f, 25.0f } };
		NumericAttribute<float>		frequency_chance =	{ this, "FrequencyChance", 0.0f, 0.0f, 1.0f };
		NumericAttribute<float>		offset_chance =		{ this, "OffsetChance", 1.0f, 0.0f, 1.0f };

		Link colorComp = { *this };

	private:
		float mCurrentOffset = 0.0f;
		float mNewOffOffset  = 0.0f;
		float mCurrentVel = 0.0f;
	};

}

RTTI_DECLARE_BASE(nap::GrainModComponent)
RTTI_DECLARE(nap::GrainColorModComponent)
