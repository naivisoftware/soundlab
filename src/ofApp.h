#pragma once

#include "ofMain.h"
#include "ofMain.h"
#include "ofxGui.h"

// Nap Includes
#include <nap/core.h>
#include <nap/patch.h>

#include <Lib/Audio/Unit/AudioDevice.h>
#include <Lib/Audio/Utility/AudioFile/AudioFileService.h>
#include <Lib/Audio/Unit/Granular/Granulator.h>
#include <openFrameworks/Gui/OFControlPanel.h>

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


class ofApp : public ofBaseApp
{
public:
	void						setup();
	void						update();
	void						draw();
	void						exit();

	void						keyPressed(int key);
	void						keyReleased(int key);
	void						mouseMoved(int x, int y);
	void						mouseDragged(int x, int y, int button);
	void						mousePressed(int x, int y, int button);
	void						mouseReleased(int x, int y, int button);
	void						mouseEntered(int x, int y);
	void						mouseExited(int x, int y);
	void						windowResized(int w, int h);
	void						dragEvent(ofDragInfo dragInfo);
	void						gotMessage(ofMessage msg);
	void						audioOut(float * output, int bufferSize, int nChannels) override;
		
private:
	// Services
	nap::OFService*				mOFService;
	nap::EtherDreamService*		mLaserService;
	nap::Core					mCore;

	// Camera
	void						createCameraEntity();
	void						createLaserEntity();
	void						flipProjectionMethod();
	void						resetCamera();
	void						createAudio();
	nap::Entity*				mCamera = nullptr;			//< Scene camera
	nap::Entity*				mLaserEntity = nullptr;		//< Laser
	nap::Entity*				mAudioEntity = nullptr;		//< Audio
	nap::Entity*				mSplineEntity = nullptr;	//< Active Spline

	// Sound
	ofSoundStream soundStream;
	lib::audio::AudioService* audioService = nullptr;
	lib::gui::OFControlPanel granulatorPanel;
    lib::gui::OFControlPanel resonatorPanel;
    std::unique_ptr<AudioComposition> audioComposition = nullptr;

	// Hook up
	void grainTriggered(lib::TimeValue& time, const lib::audio::GrainParameters& params);
    nap::Slot<lib::TimeValue, const lib::audio::GrainParameters&> mGrainTriggered = { [&](lib::TimeValue time, const lib::audio::GrainParameters& params)
	{
		grainTriggered(time, params);
    }};
};
