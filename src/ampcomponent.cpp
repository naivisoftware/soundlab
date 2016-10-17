#include <ampcomponent.h>
#include <nap/entity.h>
#include <Lib/Audio/Unit/AudioDevice.h>
#include <Utils/nofUtils.h>
#include <napoftransform.h>

namespace nap
{
	/**
	@brief Constructor
	**/
	AmpIntensityComponent::AmpIntensityComponent()
	{
		added.connect(mAdded);
		colorComponent.setTargetType(RTTI_OF(OFSplineColorComponent));
		mEnableUpdates.setValue(false);
	}


	/**
	@brief Modulates intensity value of spline based on rms amplitude
	**/
	void AmpIntensityComponent::onUpdate()
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
	void AmpIntensityComponent::onAdded(const Object& obj)
	{
		mAudioService = this->getParent()->getCore().getService<lib::audio::AudioService>();
		if (mAudioService == nullptr)
		{
			nap::Logger::warn(*this, "Unable to find audio service!");
		}
	}


	//////////////////////////////////////////////////////////////////////////


	/**
	@brief Connect
	**/
	AmpScaleComponent::AmpScaleComponent()
	{
		added.connect(mAdded);
		transformLink.setTargetType(RTTI_OF(OFTransform));
		mEnableUpdates.setValue(false);
	}


	/**
	@brief Scales uniform scale based on sampled v
	**/
	void AmpScaleComponent::onUpdate()
	{
		if (mAudioService == nullptr)
			return;

		if (!transformLink.isLinked())
			return;

		// Calculate value
		float amp = mAudioService->rmsAmplitude.getValue();
		amp = gFit(amp, range.getValue().x, range.getValue().y, 0.0f, 1.0f);

		// Calculate active range
		float range = scale.getValue() * influence.getValue();

		// Now add value based 
		float dv(0.0f);
		switch (mode.getValue())
		{
		case 0:
			dv = range * amp;
			break;
		case 1:
			dv = ofLerp(0.0f - (range / 2.0f), range / 2.0f, amp);
			break;
		case 2:
			dv = 0.0f - (range * amp);
			break;
		}

		// Add start value
		dv += start.getValue();

		// Blend
		mCurrentAmp = gSmoothDamp(mCurrentAmp, dv, mCurrentVel, damping.getValue(), 1000.0f, ofGetLastFrameTime());

		// Set
		OFTransform* scale_comp = transformLink.getTarget<OFTransform>();
		scale_comp->mUniformScale.setValue(mCurrentAmp);
	}


	/**
	@brief Set the audio service
	**/
	void AmpScaleComponent::onAdded(const Object& obj)
	{
		mAudioService = this->getParent()->getCore().getService<lib::audio::AudioService>();
		if (mAudioService == nullptr)
		{
			nap::Logger::warn(*this, "Unable to find audio service!");
		}
	}

}

RTTI_DEFINE(nap::AmpIntensityComponent)
RTTI_DEFINE(nap::AmpScaleComponent)