#pragma once

#include "ofMain.h"
#include "ofMain.h"
#include "ofxGui.h"

// Nap Includes
#include <nap/core.h>
#include <nap/patch.h>

// Audio Includes
#include <Lib/Audio/Unit/AudioService.h>
#include <Lib/Audio/Utility/AudioFile/AudioFileService.h>
#include <Lib/Audio/Unit/Granular/Granulator.h>
#include <Lib/Utility/Scheduler/SchedulerService.h>

// OF Includes
#include <openFrameworks/Gui/OFControlPanel.h>
#include <Utils/nofattributewrapper.h>
#include <audio.h>

namespace nap
{
	class OFService;
	class EtherDreamService;
	struct Preset;
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
	lib::audio::AudioService* getAudioService() { return audioService; }
	nap::Entity*						getSession()			{ return mSessionEntity; }
	nap::Preset*						getCurrentPreset()		{ return mCurrentPreset; }
	nap::Entity*						getAutomation()			{ return mAutomationEntity; }

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
	void								createAutomation();
	
	// Utility for dragging
	ofVec3f								mStartCoordinates;
	ofVec3f								mOffsetCoordinates;

	nap::Entity*						mCamera = nullptr;				//< Scene camera
	nap::Entity*						mLaserEntity = nullptr;			//< Laser
	nap::Entity*						mAudioEntity = nullptr;			//< Audio
	nap::Entity*						mSplineEntity = nullptr;		//< Active Spline
	nap::Entity*						mSessionEntity = nullptr;		//< Active session
	nap::Entity*						mAutomationEntity = nullptr;	//< Automation Entity

	// Sound
	ofSoundStream soundStream;
	lib::audio::AudioService*			audioService = nullptr;
    lib::SchedulerService*              schedulerService = nullptr;
    std::unique_ptr<AudioComposition>	audioComposition = nullptr;

	// Gui + Serialization
	Gui*								mGui;
	nap::Preset*						mCurrentPreset = nullptr;

	void								setupGui();
	void								presetIndexChanged(const int& idx);
	void								seedChanged(const int& value);
	NSLOT(mPresetChanged, const int&,	presetIndexChanged)
	NSLOT(mSeedChanged, const int&,		seedChanged)

	// Hook up
	void grainTriggered(lib::TimeValue& time, const lib::audio::GrainParameters& params);
    nap::Slot<lib::TimeValue, const lib::audio::GrainParameters&> mGrainTriggered = { [&](lib::TimeValue time, const lib::audio::GrainParameters& params)
	{
		grainTriggered(time, params);
    }};
};
