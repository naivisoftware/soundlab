#include <grainmodcomponent.h>
#include <Utils/nofUtils.h>

namespace nap
{
	// Default constructor
	GrainModComponent::GrainModComponent()
	{
		mPreviousTriggerTime = ofGetElapsedTimef();
		mEnableUpdates.setValue(false);
	}


	void nap::GrainModComponent::onUpdate()
	{
		// Lock for reading
		mMutex.lock();

		// Initialize default values
		bool trigger_received = false;
		lib::TimeValue time(0);
		lib::audio::GrainParameters params;

		// Copy values thread safe
		if (mTriggered)
		{
			trigger_received = true;
			params = mTriggerParameters;
			time = mTriggerTime;
			mTriggered = false;
		}

		// Unlock
		mMutex.unlock();

		// If we haven't received a new trigger, continue
		if (!trigger_received)
		{
			updateParameter();
			return;
		}

		// Otherwise check if we are allowed to trigger
		float current_time = ofGetElapsedTimef();
		float time_diff = current_time - mPreviousTriggerTime;
		bool in_time_range = current_time - mPreviousTriggerTime > cutoff.getValue();
		bool random_allowed = ofRandom(1.0f) < acceptance.getValue();

		// Trigger if allowed
		if (in_time_range && random_allowed)
		{
			mPreviousTriggerTime = current_time;
			triggered(time, params, time_diff);
		}

		// Update parameter
		updateParameter();
	}


	// Registers the grain signal
	void nap::GrainModComponent::registerGrainSignal(AudioComposition& inComposition)
	{
		inComposition.getGrainSignalForPlayer(0).connect(mGrainTriggered);
	}


	// Triggers the underlying base class
	void nap::GrainModComponent::grainTriggered(lib::TimeValue& time, const lib::audio::GrainParameters& params)
	{
		mMutex.lock();
		mTriggered = true;
		mTriggerTime = time;
		mTriggerParameters = params;
		mMutex.unlock();
	}


	//////////////////////////////////////////////////////////////////////////


	void nap::GrainColorModComponent::updateParameter()
	{
		// Make sure we have a color comp
		if (!colorComp.isLinked())
		{
			nap::Logger::warn(*this, "no valid color component found to set");
			return;
		}

		// Calculate new offset
		mCurrentOffset = gSmoothDamp(mCurrentOffset, mNewOffOffset, mCurrentVel, damp.getValue(), 1000.0f, ofGetLastFrameTime());

		OFSplineColorComponent* color_comp = colorComp.getTarget<OFSplineColorComponent>();
		color_comp->mOffset.setValue(mCurrentOffset);
	}



	GrainColorModComponent::GrainColorModComponent() : GrainModComponent()
	{
		colorComp.setTargetType(RTTI_OF(OFSplineColorComponent));
	}


	// Occurs when triggered
	void GrainColorModComponent::triggered(lib::TimeValue& time, const lib::audio::GrainParameters& params, float timeDiff)
	{
		// Set new offset if allowed
		if (ofRandom(1.0f) < offset_chance.getValue())
		{
			mNewOffOffset = ofRandom(1.0f);
		}

		// Make sure we have a color comp
		if (!colorComp.isLinked())
		{
			nap::Logger::warn(*this, "no valid color component found to set");
			return;
		}

		// Calculate new frequency value
		if (ofRandom(1.0f) < frequency_chance.getValue())
		{
		  float new_f = ofRandom(frequency_range.getValue().x, frequency_range.getValue().y);
		  OFSplineColorComponent* color_comp = colorComp.getTarget<OFSplineColorComponent>();
		  color_comp->mFrequency.setValue(new_f);
		}
	}
}



RTTI_DEFINE(nap::GrainModComponent)
RTTI_DEFINE(nap::GrainColorModComponent)
