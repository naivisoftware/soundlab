#include <splineutils.h>
#include <napofsplinecomponent.h>
#include <napoftransform.h>
#include <napoftracecomponent.h>
#include <napoflagcomponent.h>
#include <napofsplinemodulationcomponent.h>
#include <napofsimpleshapecomponent.h>

// Const
const ofFloatColor gDefaultSplineColor(1.0f, 1.0f, 1.0f);
const ofFloatColor gDefaultBlackColor(0.09f, 0.52f, 0.388f, 1.0f);
const ofFloatColor gSwitchColor(0.09f, 0.52f, 0.388f, 1.0f);

static const std::string	sTraceLengthLagName("TraceLengthLag");
static const std::string	sTraceLagSpeedName("TraceSpeedLag");
static const std::string	sLFOAmplitudeLagName("LFOAmpLag");
static const std::string	sLFOSpeedLagName("LFOSpeedLag");
static const std::string	sLFOFreqLagName("LFOFreqLag");
static const std::string	sColorSpeedName("ColorSpeedLag");
static const std::string	sScaleLagName("ScaleLag");
static const std::string	sOffsetLagName("TraceOffsetLag");

// Creates a new spline
nap::Entity& addSpline(nap::Entity& parent, const ofVec3f& worldPos)
{
	// Create entity
	nap::Entity& new_entity = parent.addEntity("new_spline");

	// Add Default components
	nap::OFSplineComponent& spline_component = new_entity.addComponent<nap::OFSplineComponent>();
	nap::OFSplineSelectionComponent& ss_compomnent = new_entity.addComponent<nap::OFSplineSelectionComponent>();
	nap::OFTransform& tx_component = new_entity.addComponent<nap::OFTransform>();
	nap::OFSplineColorComponent& col_component = new_entity.addComponent<nap::OFSplineColorComponent>();
	nap::OFTextComponent& text_component = new_entity.addComponent<nap::OFTextComponent>();
	nap::OFRotateComponent& rotate_component = new_entity.addComponent<nap::OFRotateComponent>();
	//nap::OFSplineLFOModulationComponent& lfo_component = new_entity.addComponent<nap::OFSplineLFOModulationComponent>();
	nap::OFTraceComponent& trace_component = new_entity.addComponent<nap::OFTraceComponent>();
	nap::OFSplineUpdateGPUComponent& gpu_component = new_entity.addComponent<nap::OFSplineUpdateGPUComponent>();
	nap::OFSplineFromFileComponent& file_component = new_entity.addComponent<nap::OFSplineFromFileComponent>();

	// Add material
	nap::OFMaterial& spline_mat = new_entity.addComponent<nap::OFMaterial>();
	spline_mat.mShader.setValue("shaders/spline_shader");
	assert(spline_mat.isLoaded());

	/*
	// Add Lag components
	nap::OFFloatLagComponent& trace_lag = new_entity.addComponent<nap::OFFloatLagComponent>(sTraceLengthLagName);
	trace_lag.SetAttribute(trace_component.mLength);
	trace_lag.mSmoothTime.setValue(0.2f);

	nap::OFFloatLagComponent& trace_speed_lag = new_entity.addComponent<nap::OFFloatLagComponent>(sTraceLagSpeedName);
	trace_speed_lag.SetAttribute(trace_component.mSpeed);
	trace_speed_lag.mSmoothTime.setValue(0.25f);
	trace_speed_lag.mTargetValue.setValue(1.0f);
	*/

	/*
	nap::OFFloatLagComponent& lfo_speed_lag = new_entity.addComponent<nap::OFFloatLagComponent>(sLFOSpeedLagName);
	lfo_speed_lag.SetAttribute(lfo_component.mSpeed);
	lfo_speed_lag.mSmoothTime.setValue(0.05f);
	lfo_speed_lag.mMaxSpeed.setValue(1000.0f);


	nap::OFFloatLagComponent& lfo_amp_lag = new_entity.addComponent<nap::OFFloatLagComponent>(sLFOAmplitudeLagName);
	lfo_amp_lag.SetAttribute(lfo_component.mAmplitude);
	lfo_amp_lag.mSmoothTime.setValue(0.025f);
	lfo_amp_lag.mMaxSpeed.setValue(5000.0f);

	nap::OFFloatLagComponent& lfo_freq_lag = new_entity.addComponent<nap::OFFloatLagComponent>(sLFOFreqLagName);
	lfo_freq_lag.SetAttribute(lfo_component.mFrequency);
	lfo_freq_lag.mSmoothTime.setValue(0.5f);
	*/

	//nap::OFFloatLagComponent& color_speed_lag = new_entity.addComponent<nap::OFFloatLagComponent>(sColorSpeedName);
	//color_speed_lag.SetAttribute(col_component.mCycleSpeed);
	//color_speed_lag.mSmoothTime.setValue(0.5);

	//nap::OFVec3LagComponent& scale_lag = new_entity.addComponent<nap::OFVec3LagComponent>(sScaleLagName);
	//scale_lag.SetAttribute(tx_component.mScale);
	//scale_lag.mSmoothTime.setValue(0.35f);

	//nap::OFFloatLagComponent& trace_offset_lag = new_entity.addComponent<nap::OFFloatLagComponent>(sOffsetLagName);
	//trace_offset_lag.SetAttribute(trace_component.mOffset);
	//trace_offset_lag.mSmoothTime.setValue(0.075f);

	// Set transform component
	tx_component.mTranslate.setValue(worldPos);

	// Set shape
	//int itype = mGui.mShapeSelection;
	SplineType stype = SplineType::Hexagon;

	// Set spline type
	//ss_compomnent.mSplineCount.setValue(500);
	//ss_compomnent.mSplineSize.setValue(200.0f);
	//ss_compomnent.mSplineType.setValue(stype);

	// Test
	file_component.mFile.setValue("svg/open.svg");

	// Set colors
	col_component.ClearColors();
	col_component.AddColor(gDefaultSplineColor);
	col_component.AddColor(ofFloatColor(1.0f, 0.0f, 0.0f, 1.0f));
	col_component.AddColor(gDefaultSplineColor);

	trace_component.mLength.setValue(0.1f);
	trace_component.mSpeed.setValue(0.1f);

	//col_component.mCycleSpeed.setValue(0.05f);
	col_component.update();
	col_component.mEnableUpdates.setValue(true);


	// Set trace point count
	trace_component.mCount.setValue(500);

	// Set text
	text_component.mText.setValue(new_entity.getName());

	// Set rotation
	rotate_component.mX.setValue(0.175f);
	rotate_component.mY.setValue(0.125f);
	rotate_component.mZ.setValue(0.1f);
	rotate_component.mSpeed.setValue(100.0f);

	// Set frequency
	//lfo_component.mFrequency.setValue(10.0f);

	// Store 
	// mSplineEntities.emplace_back(&new_entity);

	// Emit signal
	// SplineAdded.trigger(new_entity);

	return new_entity;
}