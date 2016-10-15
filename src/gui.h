#pragma once

#include <ofApp.h>
#include <Utils/nofattributewrapper.h>

class Gui
{
public:
	Gui(ofApp& inApp) : mApp(inApp)	{ }

	void Setup();
	void Draw();
	void Position(int screenWidth, int screenHeight);

	// All parameters as attribute wrappers
	OFAttributeWrapper			mColorParameters;
	OFAttributeWrapper			mXformParameters;
	OFAttributeWrapper			mRotateParameters;
	OFAttributeWrapper			mScaleParameters;
	OFAttributeWrapper			mTraceParameters;
	OFAttributeWrapper			mLFOParameters;
	OFAttributeWrapper			mSelectionParameters;
	OFAttributeWrapper			mFileParameters;
	OFAttributeWrapper			mLaserServiceParameters;
	OFAttributeWrapper			mLaserCamParameters;
	OFAttributeWrapper			mGranulatorParameters;

	// All gui wrappers
	ofxPanel					mSplineGui;
	ofxPanel					mLaserGui;
	std::vector<std::unique_ptr<ofxPanel>> audioGuis;

private:
	// Reference to the app
	ofApp& mApp;
};
