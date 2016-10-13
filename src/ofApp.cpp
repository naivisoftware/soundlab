#include "ofApp.h"
#include "ofSoundBuffer.h"

// nap
#include <nap/coremodule.h> // niet weghalen! OSX

// OF Includes
#include <napofservice.h>
#include <napofshaderbinding.h>
#include <settings.h>

// Etherdream Includes
#include <napetherservice.h>
#include <napethercamera.h>
#include <napofsimpleshapecomponent.h>
#include <napofimagecomponent.h>
#include <napofattributes.h>
#include <napofsplinemodulationcomponent.h>

// Utils
#include <splineutils.h>
#include <Utils/nofUtils.h>

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

	//////////////////////////////////////////////////////////////////////////
	// TEST
	//////////////////////////////////////////////////////////////////////////

	// Create spline entity and add first spline
	nap::Entity& spline_e = mCore.addEntity("Splines");
	mSplineEntity = &addSpline(spline_e, { 0.0f, 0.0f, 0.0f});

	// Set as entity to draw
 	nap::EtherDreamCamera* ether_cam = mLaserEntity->getComponent<nap::EtherDreamCamera>();
 	assert(ether_cam != nullptr);
 	ether_cam->mRenderEntity.setTarget(*mSplineEntity);

	//////////////////////////////////////////////////////////////////////////
	// GUI
	//////////////////////////////////////////////////////////////////////////

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

	// Draw Gui
	ofEnableAlphaBlending();
	mSplineGui.draw();
	mLaserGui.draw();
    audioComposition->getGuiForPlayer(0).draw();
    audioComposition->getGuiForPlayer(1).draw();
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
void ofApp::windowResized(int w, int h)
{
	int spacing = gGetAppSetting<int>("GuiSpacing", 25);

	// Position spline gui
	ofPoint current_point(10, 10);
	mSplineGui.setPosition(current_point);
	current_point.x += mSplineGui.getWidth() + spacing;

	// Position laser gui
	mLaserGui.setPosition(current_point);

	// Position audio gui 1
	current_point.x = w - (audioComposition->getGuiForPlayer(0).getWidth() * 2) - (spacing * 2);
	audioComposition->getGuiForPlayer(0).setPosition(current_point);

	// Position audio gui 2
	current_point.x += (audioComposition->getGuiForPlayer(1).getWidth() + spacing);
	audioComposition->getGuiForPlayer(1).setPosition(current_point);
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
    mCore.addService<AudioFileService>();

	audioService = &mCore.addService<AudioService>();
	audioService->setBufferSize(64);
	audioService->setSampleRate(44100);
	audioService->setActive(true);

    audioComposition = make_unique<AudioComposition>(mCore.getRoot(), ofFile("audiosettings.json").getAbsolutePath());

	// Connect to sound device
	soundStream.printDeviceList();
	int sound_id = gGetAppSetting<int>("SoundDeviceID", 1);
	nap::Logger::info("opening sound device: %d", sound_id);
	soundStream.setDeviceID(sound_id);

	soundStream.setup(this, 2, 0, audioService->getSampleRate(), 256, 4);

}


// Test for figuring out auto mapping of objects later on
void ofApp::setupGui()
{
	// Default gui setip
	ofxGuiSetFont(gGetAppSetting<std::string>("GuiFont", "Arial"), gGetAppSetting<int>("GuiFontSize", 8));

	// Populate parameters for spline modulation
	mColorParameters.setName("Color");
	mColorParameters.addObject(*(mSplineEntity->getComponent<nap::OFSplineColorComponent>()));

	mRotateParameters.setName("Rotation");
	mRotateParameters.addObject(*(mSplineEntity->getComponent<nap::OFRotateComponent>()));
	
	mScaleParameters.setName("Scale");
	mScaleParameters.addObject(*(mSplineEntity->getComponent<nap::OFScaleComponent>()));
	mScaleParameters.addObject(*(mSplineEntity->getComponent<nap::OFTransform>()));

	mTraceParameters.setName("Tracer");
	mTraceParameters.addObject(*(mSplineEntity->getComponent<nap::OFTraceComponent>()));

	mLFOParameters.setName("LFO");
	mLFOParameters.addObject(*(mSplineEntity->getComponent<nap::OFSplineLFOModulationComponent>()));

	mSelectionParameters.setName("Selection");
	mSelectionParameters.addObject(*(mSplineEntity->getComponent<nap::OFSplineSelectionComponent>()));

	mFileParameters.setName("Spline File");
	mFileParameters.addObject(*(mSplineEntity->getComponent<nap::OFSplineFromFileComponent>()));

	// Add parameters to gui
	mSplineGui.setup();
	mSplineGui.add(mColorParameters.getGroup());
	mSplineGui.add(mRotateParameters.getGroup());
	mSplineGui.add(mScaleParameters.getGroup());
	mSplineGui.add(mTraceParameters.getGroup());
	mSplineGui.add(mLFOParameters.getGroup());
	mSplineGui.add(mSelectionParameters.getGroup());
	mSplineGui.add(mFileParameters.getGroup());
	mSplineGui.minimizeAll();

	//////////////////////////////////////////////////////////////////////////

	// Populate parameters for laser service
	mLaserServiceParameters.setName("LaserService");
	mLaserServiceParameters.addObject(*mLaserService);

	mLaserCamParameters.setName("LaserCamera");
	mLaserCamParameters.addObject(*(mLaserEntity->getComponent<nap::EtherDreamCamera>()));

	mLaserGui.setup();
	mLaserGui.add(mLaserCamParameters.getGroup());
	mLaserGui.add(mLaserServiceParameters.getGroup());
	mLaserGui.minimizeAll();
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
