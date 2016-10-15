#pragma once

#include "ofMain.h"
#include "ofMain.h"
#include "ofxGui.h"

// Nap Includes
#include <nap/core.h>
#include <nap/patch.h>

// Audio Includes
#include <Lib/Audio/Unit/AudioDevice.h>
#include <Lib/Audio/Utility/AudioFile/AudioFileService.h>
#include <Lib/Audio/Unit/Granular/Granulator.h>

// OF Includes
#include <openFrameworks/Gui/OFControlPanel.h>
#include <Utils/nofattributewrapper.h>
#include <audio.h>

namespace nap
{
	class OFService;
	class EtherDreamService;
}

namespace lib
{
	namespace audio
	{
		class Granulator;
	}
}

class Gui;

class ofApp : public ofBaseApp
{
public:
	void								setup();
	void								update();
	void								draw();
	void								exit();

	void								keyPressed(int key);
	void								keyReleased(int key);
	void								mouseMoved(int x, int y);
	void								mouseDragged(int x, int y, int button);
	void								mousePressed(int x, int y, int button);
	void								mouseReleased(int x, int y, int button);
	void								mouseEntered(int x, int y);
	void								mouseExited(int x, int y);
	void								windowResized(int w, int h);
	void								dragEvent(ofDragInfo dragInfo);
	void								gotMessage(ofMessage msg);
	void								audioOut(float * output, int bufferSize, int nChannels) override;
	
	// Utility
	nap::Entity*						getCamera()				{ return mCamera; }
	nap::Entity*						getLaser()				{ return mLaserEntity; }
	nap::Entity*						getAudio()				{ return mAudioEntity; }
	nap::Entity*						getSpline()				{ return mSplineEntity; }
	nap::EtherDreamService*				getLaserService()		{ return mLaserService; }
	AudioComposition*					getAudioComposition()	{ return audioComposition.get(); }
	nap::Entity*						getSession()			{ return mSessionEntity; }

private:
	// Services
	nap::OFService*						mOFService;
	nap::EtherDreamService*				mLaserService;
	nap::Core							mCore;

	// Camera and Laser
	void								createCameraEntity();
	void								createLaserEntity();
	void								flipProjectionMethod();
	void								resetCamera();
	void								createAudio();
	void								createSpline();
	void								createSession();
	
	// Utility for dragging
	ofVec3f								mStartCoordinates;
	ofVec3f								mOffsetCoordinates;

	nap::Entity*						mCamera = nullptr;			//< Scene camera
	nap::Entity*						mLaserEntity = nullptr;		//< Laser
	nap::Entity*						mAudioEntity = nullptr;		//< Audio
	nap::Entity*						mSplineEntity = nullptr;	//< Active Spline
	nap::Entity*						mSessionEntity = nullptr;	//< Active session

	// Sound
	ofSoundStream soundStream;
	lib::audio::AudioService*			audioService = nullptr;
    std::unique_ptr<AudioComposition>	audioComposition = nullptr;

	// Gui + Serialization
	Gui*								mGui;
	void								setupGui();

	// Hook up
	void grainTriggered(lib::TimeValue& time, const lib::audio::GrainParameters& params);
    nap::Slot<lib::TimeValue, const lib::audio::GrainParameters&> mGrainTriggered = { [&](lib::TimeValue time, const lib::audio::GrainParameters& params)
	{
		grainTriggered(time, params);
    }};
};
