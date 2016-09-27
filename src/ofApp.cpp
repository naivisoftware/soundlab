#include "ofApp.h"
#include "ofSoundBuffer.h"

// OF Includes
#include <napofservice.h>
#include <napofshaderbinding.h>

// Etherdream Includes
#include <napetherservice.h>
#include <napethercamera.h>
#include <napofsimpleshapecomponent.h>
#include <napofimagecomponent.h>
#include <napofattributes.h>

// Utils
#include <splineutils.h>
#include <Utils/nofUtils.h>

// Audio
#include <Lib/Audio/Utility/AudioFile/WavReader.h>
#include <Lib/Audio/Unit/OutputUnit.h>
#include <Lib/Audio/Unit/Granular/Granulator.h>
#include <Lib/Utility/Data/RampSequencer.h>
#include <Lib/Utility/Data/Sequencer.h>

using namespace lib;
using namespace lib::audio;
using namespace nap;

// Setup
void ofApp::setup()
{
	mOFService = &mCore.addService<nap::OFService>();
	mLaserService = &mCore.addService<nap::EtherDreamService>();

	// HACK, shouldn't be here
	nap::registerOfShaderBindings();

	// Add camera
	createCameraEntity();

	// Add laser entity
	createLaserEntity();

	// Create audio service + devices
	createAudio();

	// Create spline entity and add first spline
	nap::Entity& spline_e = mCore.addEntity("Splines");
	nap::Entity& spline_entity = addSpline(spline_e, { 0.0f, 0.0f, 0.0f});

	// Set as entity to draw
	nap::EtherDreamCamera* ether_cam = mLaserEntity->getComponent<nap::EtherDreamCamera>();
	assert(ether_cam != nullptr);
	ether_cam->mRenderEntity.setTarget(spline_entity);
}


// Update
void ofApp::update()
{
	mOFService->update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	mOFService->draw();
	mLaserService->draw();

	// Draw framerate
	float rate = ofGetFrameRate();
	ofColor ccolor = rate < 55 ? ofColor::red : ofColor::green;
	ofSetColor(ccolor);
	ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, ofGetWindowHeight() - 10);

	// Draw audio panel
	granulatorPanel.draw();

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
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

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
void ofApp::windowResized(int w, int h){

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
	color_attr.setValue({ 1.0f, 0.0f, 0.0f });

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


void ofApp::resetCamera()
{
	mCamera->getComponent<nap::OFSimpleCamComponent>()->reset();
}



void ofApp::createAudio()
{
	float bufferSampleRate;
	readWavFile(ofFile("mydyingbride.wav").getAbsolutePath(), mBuffer, bufferSampleRate);
	mBufferStream = std::make_unique<AudioBufferStream>(bufferSampleRate, mBuffer);

	audioService = &mCore.addService<AudioService>();
	audioService->setBufferSize(64);
	audioService->setSampleRate(44100);
	audioService->setActive(true);

	auto& eSound = mCore.addEntity("sound");
	auto& patchComponent = eSound.addComponent<PatchComponent>("patch");

	granulator = &patchComponent.getPatch().addOperator<Granulator>("granulator");
	granulator->channelCount.setValue(2);
	granulator->setInputStream(mBufferStream.get());
    granulator->density.setProportion(0.5);
    granulator->duration.setValue(100);
    granulator->irregularity.setProportion(0.5);
    granulator->positionDev.setProportion(0.25);
    granulator->positionSpeed.setProportion(0.4);

	auto& output = patchComponent.getPatch().addOperator<OutputUnit>("output");
	output.channelCount.setValue(2);
	output.audioInput.connect(granulator->output);

//	auto& animator = patchComponent.getPatch().addOperator<RampSequencer>("animator");
//	animator.sequence.setValue({ 0.2f, 0.5f, 0.1f, 0.6f });
//	animator.durations.setValue({ 3000, 500, 6000 });
//	animator.looping.setValue(true);
//	animator.schedulerInput.connect(output.schedulerOutput);
//	granulator->density.proportionPlug.connect(animator.output);

//    granulator->playCloudParams({ { "pitch", 1. }, { "position", 0.5 }, { "duration", 2000 },  { "amplitude", 1 }, { "attack", 1000 }, { "decay", 1000 }});

    FloatArray pitches1 = { 1.0f, 1.5f, 4/3.f, 5/3.f, 7/6.f, 2/3.f };
    FloatArray pitches2 = mult({ 1.5f, 4/3.f, 5/3.f, 7/6.f, 5/6.f }, 1.5f);
    
    auto& animator = patchComponent.getPatch().addOperator<Sequencer>("animator");
    animator.sequences.addAttribute<FloatArray>("pitch", pitches1);
    animator.sequences.addAttribute<FloatArray>("position", { 0.2f } );
    animator.sequences.addAttribute<float>("attack", 500);
    animator.sequences.addAttribute<float>("decay", 500);
    animator.sequences.addAttribute<float>("duration", 2000);
    animator.times.setValue({ 2000, 1000, 1000, 2000, 1000 });
    animator.looping.setValue(true);
    animator.schedulerInput.connect(output.schedulerOutput);
    granulator->cloudInput.connect(animator.output);
    
    auto& animator2 = patchComponent.getPatch().addOperator<Sequencer>("animator2");
    animator2.sequences.addAttribute<FloatArray>("pitch", pitches2);
    animator2.sequences.addAttribute<FloatArray>("position", { 0.2f } );
    animator2.sequences.addAttribute<float>("attack", 500);
    animator2.sequences.addAttribute<float>("decay", 500);
    animator2.sequences.addAttribute<float>("duration", 3000);
    animator2.times.setValue({ 3000, 1500, 3000, 1500, 3000 });
    animator2.looping.setValue(true);
    animator2.schedulerInput.connect(output.schedulerOutput);
    granulator->cloudInput.connect(animator2.output);
    
	granulatorPanel.setup();
	granulatorPanel.setControlManager(granulator->getControlManager());

	soundStream.printDeviceList();

	soundStream.setup(this, 2, 0, audioService->getSampleRate(), 256, 4);


}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
