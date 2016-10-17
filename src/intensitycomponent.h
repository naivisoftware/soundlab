#pragma once

#include <nap/component.h>
#include <nap/coremodule.h>
#include <napofupdatecomponent.h>
#include <ofVec2f.h>
#include <napofattributes.h>

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
	class IntensityComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		IntensityComponent() = default;
		
		// Parameters
		NumericAttribute<float>		influence	{ this, "Influence", 0.0f, 0.0f, 1.0f };
		NumericAttribute<ofVec2f>	range		{ this, "Range", {0.0f, 1.0f}, {0.0f, 0.001f}, {1.0f, 1.0f} };

		// Update call
		virtual void onUpdate() override;

	private:
		// Audio service
		lib::audio::AudioService* mAudioService = nullptr;
	};
}

RTTI_DECLARE(nap::IntensityComponent)