#include "ofApp.h"
#include "ofSoundBuffer.h"

// nap
#include <nap/coremodule.h> // niet weghalen! OSX

// OF Includes
#include <napofservice.h>
#include <napofshaderbinding.h>
#include <settings.h>
#include <presetcomponent.h>

// Etherdream Includes
#include <napetherservice.h>
#include <napethercamera.h>
#include <napofsimpleshapecomponent.h>
#include <napofimagecomponent.h>
#include <napofattributes.h>
#include <napofsplinemodulationcomponent.h>
#include <intensitycomponent.h>

// Utils
#include <splineutils.h>
#include <Utils/nofUtils.h>
#include <4dService/SpatialService.h>
#include <settingserializer.h>

// Gui
#include <gui.h>
#include <assert.h>

using namespace lib;
using namespace lib::audio;
using namespace nap;

// Setup
void ofApp::setup()
{
	mOFService = &mCore.addService<nap::OFService>();
 	mLaserService = &mCore.addService<nap::EtherDreamService>();
	audioService = &mCore.addService<AudioService>();

	// HACK, shouldn't be here
	nap::registerOfShaderBindings();

	// Add camera
	createCameraEntity();

	// Add laser entity
 	createLaserEntity();

	// Create audio service + devices
	createAudio();

	// Create initial spline
	createSpline();

	// Create session
	createSession();

	// Create automation
	createAutomation();

	// Setup gui (always last)
	setupGui();
}


// Update
void ofApp::update()
{
	mOFService->update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Draw services
	mOFService->draw();
	mLaserService->draw();

	// Draw framerate
	float rate = ofGetFrameRate();
	ofColor ccolor = rate < 55 ? ofColor::red : ofColor::green;
	ofSetColor(ccolor);
	ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, ofGetWindowHeight() - 10);

	// Draw gui
	mGui->Draw();
}


// Kills laser
void ofApp::exit()
{
	mLaserService->Stop();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch (key)
	{
	case 'o':
		flipProjectionMethod();
		resetCamera();
		break;
	case 'r':
		resetCamera();
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case '0':
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}


/**
@brief Moves the spline based on the drag difference
**/
void ofApp::mouseDragged(int x, int y, int button)
{
	// Don't transform when move is disabled
	if (!mSessionEntity->getAttribute<bool>("MoveSpline")->getValue())
		return;

	// Otherwise move based on previous loc
	nap::OFSimpleCamComponent* cam = mCamera->getComponent<nap::OFSimpleCamComponent>();
	assert(cam != nullptr);

	if (cam->getProjectionMode() == nap::ProjectionMode::Perspective)
	{
		nap::Logger::warn("Can't move shapes in perspective mode");
		return;
	}

	nap::OFTransform* xform = mSplineEntity->getComponent<nap::OFTransform>();
	assert(xform != nullptr);

	// Get screen coordinates
	ofVec3f screen_coords(float(x), float(y), 0.0f);

	// Get world coordinates from screen
	ofVec3f world_coordinates = cam->mCamera.getValue().screenToWorld(screen_coords, ofGetCurrentViewport());
	
	// Calculate new transform
	//ofVec3f new_xform = ((world_coordinates - mStartCoordinates) + mStartCoordinates) + mOffsetCoordinates;
	ofVec3f new_xform = world_coordinates + mOffsetCoordinates;
	xform->mTranslate.setValue(new_xform);
}	

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	// only handle left mouse clicks
	if (button != OF_MOUSE_BUTTON_1)
		return;

	// get cam
	nap::OFSimpleCamComponent* cam = mCamera->getComponent<nap::OFSimpleCamComponent>();
	assert(cam != nullptr);

	// ensure we're in orthographic mode
	if (cam->getProjectionMode() == nap::ProjectionMode::Perspective)
	{
		nap::Logger::warn("Can't add or select shapes in perspective mode");
		return;
	}

	// Get screen coordinates
	nap::OFTransform* xform = mSplineEntity->getComponent<nap::OFTransform>();
	assert(xform);
	ofVec3f screen_coordinates(float(x), float(y), 0.0f);

	// Get world coordinates
	ofVec3f world_coordinates = cam->mCamera.getValue().screenToWorld(screen_coordinates, ofGetCurrentViewport());
	world_coordinates.z = xform->mTranslate.getValue().z;

	// Store world coordinates
	mStartCoordinates = world_coordinates;
	
	// Set position
	mOffsetCoordinates = xform->mTranslate.getValue() - world_coordinates;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
	mGui->Position(w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}


// Output sound gathered by output service
void ofApp::audioOut(float * output, int bufferSize, int nChannels)
{
	int i = 0;
	while (i < (bufferSize * nChannels) - 1)
	{
		audioService->processSamplesInterleaved(nullptr, &output[i], audioService->getBufferSize(), 0, nChannels);
		i += audioService->getBufferSize() * nChannels;
	}
}


// Creates the camera entity
void ofApp::createCameraEntity()
{
	// Add camera entity
	mCamera = &mCore.addEntity("Camera");
	nap::OFSimpleCamComponent& cam_comp = mCamera->addComponent<nap::OFSimpleCamComponent>("CameraComponent");
	cam_comp.setDistance(1000.0f);
	cam_comp.setFarClip(10000.0f);

	mOFService->setDefaultCamera(*mCamera);

	// Ensure orthographic mode
	flipProjectionMethod();
}


// Create laser entity
void ofApp::createLaserEntity()
{
	// Create entity
	mLaserEntity = &mCore.addEntity("Laser");

	// Add transform
	nap::OFTransform& laser_xform = mLaserEntity->addComponent<nap::OFTransform>();
	laser_xform.mTranslate.setValue(ofVec3f(0.0f, 0.0f, 0.1f));

	// Add camera
	nap::EtherDreamCamera& laser_cam = mLaserEntity->addComponent<nap::EtherDreamCamera>();

	// Add drawable projection plane and set color
	nap::OFPlaneComponent& plane = mLaserEntity->addComponent<nap::OFPlaneComponent>();

	// Add shader component
	nap::OFMaterial& material = mLaserEntity->addComponent<nap::OFMaterial>();
	material.mShader.setValue("shaders/simple_shader");

	// Load texture
	nap::OFImageComponent& img_comp = mLaserEntity->addComponent<nap::OFImageComponent>();
	img_comp.mFile.setValue("textures/laser_canvas.png");
	assert(img_comp.isAllocated());

	// Link texture
	material.addAttribute<ofTexture*>("mTexture").link(img_comp.texture);

	// Add color
	nap::Attribute<ofVec3f>& color_attr = material.addAttribute<ofVec3f>("mColor");
	color_attr.setValue({ 1.0f, 1.0f, 1.0f });

	// Connect to signal changes
	laser_cam.mFrustrumWidth.valueChangedSignal.connect(plane.mWidth.setValueSlot);
	laser_cam.mFrustrumHeight.valueChangedSignal.connect(plane.mHeight.setValueSlot);

	laser_cam.setFrustrumWidth(1000.0f);
	laser_cam.setFrustrumHeight(1000.0f);

	laser_cam.mTraceMode.setValue(false);
}


// Flips projection method
void ofApp::flipProjectionMethod()
{
	nap::OFSimpleCamComponent* cam = mCamera->getComponent<nap::OFSimpleCamComponent>();
	assert(cam != nullptr);

	switch (cam->getProjectionMode())
	{
	case nap::ProjectionMode::Perspective:
	{
		cam->mOrthographic.setValue(true);
		cam->enableMouseInput(false);
		nap::Logger::info("Enabled orthographic projection");
		break;
	}
	case nap::ProjectionMode::Orthographic:
		cam->mOrthographic.setValue(false);
		cam->enableMouseInput(true);
		nap::Logger::info("Disabled orthographic projection");
		break;
	}
}


/**
@brief Rests the camera to it's original position
**/
void ofApp::resetCamera()
{
	mCamera->getComponent<nap::OFSimpleCamComponent>()->reset();
}


/**
@brief Creates audio service and audio related entities / components
**/
void ofApp::createAudio()
{
    mCore.addService<AudioFileService>();
    mCore.addService<spatial::SpatialService>();

	audioService->setBufferSize(64);
	audioService->setSampleRate(44100);
	audioService->setActive(true);

    audioComposition = make_unique<AudioComposition>(mCore.getRoot(), ofFile("audiosettings.json").getAbsolutePath());

	// Connect to sound device
	soundStream.printDeviceList();
	int sound_id = gGetAppSetting<int>("SoundDeviceID", 1);
	nap::Logger::info("opening sound device: %d", sound_id);
	soundStream.setDeviceID(sound_id);

    int channelCount = gGetAppSetting<int>("AudioChannelCount", 2);
	soundStream.setup(this, channelCount, 0, audioService->getSampleRate(), 256, 4);

}


/**
@brief Creates the spline entity
**/
void ofApp::createSpline()
{
	// Create spline
	nap::Entity& spline_e = mCore.addEntity("Splines");
	mSplineEntity = &addSpline(spline_e, { 0.0f, 0.0f, 0.0f });

	nap::NumericAttribute<float>& warmth_attr = mSplineEntity->addChild<nap::NumericAttribute<float>>("Warmth");
	warmth_attr.setRange(0.0f, 1.0f);
	warmth_attr.setValue(0.5f);
	warmth_attr.setClamped(true);

	nap::NumericAttribute<float>& intensity_attr = mSplineEntity->addChild<nap::NumericAttribute<float>>("Intensity");
	intensity_attr.setRange(0.0f, 1.0f);
	intensity_attr.setValue(0.5f);
	intensity_attr.setClamped(true);

	// Set as entity to draw
	nap::EtherDreamCamera* ether_cam = mLaserEntity->getComponent<nap::EtherDreamCamera>();
	assert(ether_cam != nullptr);
	ether_cam->mRenderEntity.setTarget(*mSplineEntity);
}


/**
@brief Creates the session entity
**/
void ofApp::createSession()
{
	// Add move attribute for spline
	mSessionEntity = &mCore.addEntity("Session");
	nap::Attribute<bool>& move_attr = mSessionEntity->addAttribute<bool>("MoveSpline");
	move_attr.setValue(false);

	// Add preset component
	nap::PresetComponent& preset_comp = mSessionEntity->addComponent<nap::PresetComponent>("Presets");
	ofDirectory preset_dir("saves");
	preset_comp.setDirectory(preset_dir);

	// Add preset switcher
	nap::PresetSwitchComponent& preset_switcher = mSessionEntity->addComponent<nap::PresetSwitchComponent>("PresetSwitchet");

	// Connect to preset changes
	preset_comp.index.valueChangedSignal.connect(mPresetChanged);
}


/**
@brief Create automation entity + components
**/
void ofApp::createAutomation()
{
	mAutomationEntity = &mCore.addEntity("Automation");
	nap::IntensityComponent& intensity_comp =mAutomationEntity->addComponent<nap::IntensityComponent>();

	nap::Entity* spline = getSpline();
	assert(spline != nullptr);
	OFSplineColorComponent* color_comp = spline->getComponent<OFSplineColorComponent>();

	if (color_comp == nullptr)
	{
		nap::Logger::warn("color component not found, can't automate intensity");
		return;
	}
	intensity_comp.colorComponent.setTarget(*color_comp);
}


// Test for figuring out auto mapping of objects later on
void ofApp::setupGui()
{
	mGui = new Gui(*this);
	mGui->Setup();
}


/**
@brief Occurs when the preset index changes, loads a new preset from one that is already cached
**/
void ofApp::presetIndexChanged(const int& idx)
{
	nap::PresetComponent* preset_component = mSessionEntity->getComponent<nap::PresetComponent>();
	assert(preset_component != nullptr);
	mCurrentPreset = preset_component->getPreset(idx);
	assert(preset != nullptr);
	SettingSerializer serializer;
	serializer.loadSettings(*mCurrentPreset, *mGui);

	// HACK, THESE SETTINGS ARE NOT DESERIALIZED CORRECTLY
	// CAUSES A SET OF PARAMETERS TO NO BE IN THE RIGHT STATE
	nap::OFRotateComponent* rotate_comp = mSplineEntity->getComponent<nap::OFRotateComponent>();
	if (rotate_comp->mEnableUpdates.getValue())
	{
		rotate_comp->mReset.trigger();
	}
}


// Called when grains change
void ofApp::grainTriggered(lib::TimeValue& time, const lib::audio::GrainParameters& params)
{
	float mapped_v = gFit(time, 10, 1000, 0.005f, 0.45f);
	mapped_v *= ofRandom(0.25f, 1.0f);

	OFTraceComponent* trace_comp = mSplineEntity->getComponent<OFTraceComponent>();	
	trace_comp->mOffset.setValue(ofRandom(0.0f, 1.0f));
	trace_comp->mLength.setValue(mapped_v);
}


//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{}
