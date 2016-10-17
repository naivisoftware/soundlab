#include <intensitycomponent.h>
#include <nap/entity.h>
#include <Lib/Audio/Unit/AudioDevice.h>

namespace nap
{
	/**
	@brief Modulates intensity value of spline based on rms amplitude
	**/
	void IntensityComponent::onUpdate()
	{
		// Try to find the service if don't have one
		if (mAudioService == nullptr)
		{
			mAudioService = this->getParent()->getCore().getService<lib::audio::AudioService>();
		}

		// Make sure we have the audio service
		if (mAudioService == nullptr)
		{
			nap::Logger::warn(*this, "Unable to find audio service!");
			return;
		}

		float amp = mAudioService->rmsAmplitude.getValue();
//		nap::Logger::info("amplitude: %f", amp);
	}

}

RTTI_DEFINE(nap::IntensityComponent)