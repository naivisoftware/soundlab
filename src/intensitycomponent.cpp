#include <intensitycomponent.h>
#include <nap/entity.h>
#include <Lib/Audio/Unit/AudioDevice.h>
#include <Utils/nofUtils.h>

namespace nap
{
	/**
	@brief Constructor
	**/
	IntensityComponent::IntensityComponent()
	{
		added.connect(mAdded);
		colorComponent.setTargetType(RTTI_OF(OFSplineColorComponent));
		mEnableUpdates.setValue(false);
	}


	/**
	@brief Modulates intensity value of spline based on rms amplitude
	**/
	void IntensityComponent::onUpdate()
	{
		if (mAudioService == nullptr)
			return;

		if (!colorComponent.isLinked())
			return;

		// Calculate value
		float amp = mAudioService->rmsAmplitude.getValue();
		amp = gFit(amp, range.getValue().x, range.getValue().y, 0.0f, 1.0f);
		amp = ofLerp(1.0f, amp, influence.getValue()) * scale.getValue();
		
		// Apply smooth damp
		mCurrentAmp = gSmoothDamp(mCurrentAmp, amp, mCurrentVel, damping.getValue(), 1000.0f, ofGetLastFrameTime());

		if (!colorComponent.getTargetType().isKindOf(RTTI_OF(OFSplineColorComponent)))
		{
			nap::Logger::warn(*this, "invalid link type, needs to be a color component");
			return;
		}

		OFSplineColorComponent* color_comp = colorComponent.getTarget<OFSplineColorComponent>();
		color_comp->mIntensity.setValue(mCurrentAmp);
	}


	/**
	@brief Retrieves all the services necessary for this component to compute
	**/
	void IntensityComponent::onAdded(const Object& obj)
	{
		mAudioService = this->getParent()->getCore().getService<lib::audio::AudioService>();
		if (mAudioService == nullptr)
		{
			nap::Logger::warn(*this, "Unable to find audio service!");
		}
	}

}

RTTI_DEFINE(nap::IntensityComponent)