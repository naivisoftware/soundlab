#pragma once

#include <ofApp.h>
#include <Utils/nofattributewrapper.h>
#include <vector>

class Gui
{
public:
	Gui(ofApp& inApp) : mApp(inApp)	{ }

	// Sets up the gui components
	void Setup();

	// Draws all the gui components
	void Draw();

	// Positions the gui components based on width / height
	void Position(int screenWidth, int screenHeight);

	// Returns all the available guis
	const std::vector<ofxPanel*>& getGuis() const	{ return mGuis; }

private:
	// Reference to the app
	ofApp& mApp;

	// All the available guis
	std::vector<ofxPanel*>		mGuis;

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
	OFAttributeWrapper			mSessionParameters;
	OFAttributeWrapper			mPresetParameters;
	OFAttributeWrapper			mPresetAutomationParameters;
	OFAttributeWrapper			mTagParameters;

	// All gui wrappers
	ofxPanel					mSplineGui;
	ofxPanel					mLaserGui;
	ofxPanel					mSessionGui;
	std::vector<std::unique_ptr<ofxPanel>> audioGuis;

	// Load and Save buttons
	ofxButton					mLoad;
	ofxButton					mSaveAs;
	ofxButton					mSave;
	void						loadClicked();
	void						saveAsClicked();
	void						saveClicked();

	// Utility slots
	NSLOT(mSplineUpdated, const nap::Object&, splineUpdated)
	void splineUpdated(const nap::Object& obj);
};
