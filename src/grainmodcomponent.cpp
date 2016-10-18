#include <grainmodcomponent.h>
#include <napoftracecomponent.h>
#include <Utils/nofUtils.h>
#include <napofsplinemodulationcomponent.h>

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
		bool amp_allowed = params.amplitude > amp_cutoff.getValue();

		// Trigger if allowed
		if (in_time_range && random_allowed && amp_allowed)
		{
			std::cout << "grain amp: %s" << params.amplitude << "\n";

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

	// Updates the parameters
	void nap::GrainColorModComponent::updateParameter()
	{
		// Make sure we have a color comp
		if (!colorComp.isLinked())
		{
			nap::Logger::warn(*this, "no valid color component found to set");
			return;
		}

		OFSplineColorComponent* color_comp = colorComp.getTarget<OFSplineColorComponent>();

		// Calculate new offset
		mCurrentOffset = gSmoothDamp(mCurrentOffset, mNewOffOffset, mCurrentOffsetVel, offset_damp.getValue(), 1000.0f, ofGetLastFrameTime());
		color_comp->mOffset.setValue(mCurrentOffset);

		// Calculate new frequency
		mCurrentFrequency = gSmoothDamp(mCurrentFrequency, mNewFrequency, mCurrentFrequencyVel, frequency_damp.getValue(), 1000.0f, ofGetLastFrameTime());
		color_comp->mFrequency.setValue(mCurrentFrequency);
	}


	// Constructor
	GrainColorModComponent::GrainColorModComponent() : GrainModComponent()
	{
		colorComp.setTargetType(RTTI_OF(OFSplineColorComponent));
	}


	// Occurs when triggered, calculated new offset and frequency values
	void GrainColorModComponent::triggered(lib::TimeValue& time, const lib::audio::GrainParameters& params, float timeDiff)
	{
		// Set new offset if allowed
		if (ofRandom(1.0f) < offset_chance.getValue())
		{
			float amp_weight = gFit(params.amplitude, 0.0f, 0.25f, 0.15f, 1.0f);
			float tim_weight = gFit(timeDiff, 0.0f, 0.5f, 1.0f, 0.25f);
			float scale = tim_weight * amp_weight;
			float add = ofRandom(amp_weight / 2.0f, 1.0f) * scale * offset_deviation.getValue();
			mNewOffOffset = fmod(mCurrentOffset + add, 1.0f);
		}

		// Set new Frequency
		if (ofRandom(1.0f) < frequency_chance.getValue())
		{
			// Figure out weight of time
			float new_v = ofRandom(frequency_range.getValue().x, frequency_range.getValue().y);

			// Set new frequency
			mNewFrequency = new_v;
		}
	}

	//////////////////////////////////////////////////////////////////////////


	GrainTraceModComponent::GrainTraceModComponent() : GrainModComponent()
	{
		traceComp.setTargetType(RTTI_OF(OFTraceComponent));
		mEnableUpdates.valueChangedSignal.connect(mEnabled);
	}


	void GrainTraceModComponent::updateParameter()
	{
		// Make sure we have a color comp
		if (!traceComp.isLinked())
		{
			nap::Logger::warn(*this, "no valid trace component found to set");
			return;
		}

		OFTraceComponent* trace_comp = traceComp.getTarget<OFTraceComponent>();

		// Calculate new offset
		mCurrentOffset = gSmoothDamp(mCurrentOffset, mNewOffOffset, mCurrentOffsetVel, offset_damp.getValue(), 1000.0f, ofGetLastFrameTime());
		trace_comp->mOffset.setValue(mCurrentOffset);

		// Calculate new speed
		mCurrentSpeed = gSmoothDamp(mCurrentSpeed, mNewSpeed, mCurrentSpeedVel, speed_damp.getValue(), 1000.0f, ofGetLastFrameTime());
		trace_comp->mSpeed.setValue(mCurrentSpeed);

		mCurrentSize = gSmoothDamp(mCurrentSize, mNewSize, mCurrentSizeVel, size_damp.getValue(), 1000.0f, ofGetLastFrameTime());
		trace_comp->mLength.setValue(mCurrentSize);
	}


	// Sets new values if allowed
	void GrainTraceModComponent::triggered(lib::TimeValue& time, const lib::audio::GrainParameters& params, float timeDiff)
	{
		if (ofRandom(1.0f) < offset_chance.getValue())
		{
			mNewOffOffset = fmod(mCurrentOffset + ofRandom(1.0f * offset_deviation.getValue()), 1.0f);
		}

		if (ofRandom(1.0f) < speed_chance.getValue())
		{
			float scale = gFit(timeDiff, 0.0f, 0.4f, 1.0f, 0.15f);
			mNewSpeed = ofRandom(speed_range.getValue().x, speed_range.getValue().y * scale);
		}

		if (ofRandom(1.0f) < size_chance.getValue())
		{
			float scale = gFit(timeDiff, 0.0f, 0.4f, 1.0f, 0.25f);
			mNewSize = ofRandom(size_range.getValue().x, size_range.getValue().y) * scale;
		}
	}


	// Loads current values from other listening component
	void GrainTraceModComponent::enabled(const bool& v)
	{
		// Make sure we have a color comp
		if (!traceComp.isLinked())
		{
			nap::Logger::warn(*this, "no valid trace component found to fetch values from");
			return;
		}

		OFTraceComponent* trace_comp = traceComp.getTarget<OFTraceComponent>();
		mCurrentOffset = trace_comp->mOffset.getValue();
		mCurrentSize = trace_comp->mLength.getValue();
		mCurrentSpeed = trace_comp->mSpeed.getValue();
	}


	//////////////////////////////////////////////////////////////////////////


	void GrainLFOModComponent::triggered(lib::TimeValue& time, const lib::audio::GrainParameters& params, float timeDiff)
	{
		if (ofRandom(1.0f) < offset_chance.getValue())
		{
			float scale = gFit(timeDiff, 0.0f, 0.5f, 1.0f, 0.15f) * gFit(params.amplitude, 0.0f, 0.5f, 0.15f, 1.0f);
			mNewOffOffset = fmod(mCurrentOffset + ((ofRandom(0.5f, 1.0f)* scale * offset_deviation.getValue())) , 1.0f);
		}

		if (ofRandom(1.0f) < speed_chance.getValue())
		{
			float scale = gFit(timeDiff, 0.0f, 0.4f, 1.0f, 0.15f);
			mNewSpeed = ofRandom(speed_range.getValue().x, speed_range.getValue().y * scale);
		}
	}


	/**
	@brief Updates the lfo parameters
	**/
	void GrainLFOModComponent::updateParameter()
	{
		// Make sure we have a color comp
		if (!lfoComp.isLinked())
		{
			nap::Logger::warn(*this, "no valid trace component found to set");
			return;
		}

		OFSplineLFOModulationComponent* lfo_comp = lfoComp.getTarget<OFSplineLFOModulationComponent>();

		// Calculate new offset
		mCurrentOffset = gSmoothDamp(mCurrentOffset, mNewOffOffset, mCurrentOffsetVel, offset_damp.getValue(), 1000.0f, ofGetLastFrameTime());
		lfo_comp->mOffset.setValue(mCurrentOffset);

		// Calculate new speed
		mCurrentSpeed = gSmoothDamp(mCurrentSpeed, mNewSpeed, mCurrentSpeedVel, speed_damp.getValue(), 1000.0f, ofGetLastFrameTime());
		lfo_comp->mSpeed.setValue(mCurrentSpeed);
	}


	GrainLFOModComponent::GrainLFOModComponent()
	{
		lfoComp.setTargetType(RTTI_OF(OFSplineLFOModulationComponent));
	}
}


//////////////////////////////////////////////////////////////////////////





RTTI_DEFINE(nap::GrainModComponent)
RTTI_DEFINE(nap::GrainColorModComponent)
RTTI_DEFINE(nap::GrainTraceModComponent)
RTTI_DEFINE(nap::GrainLFOModComponent)
