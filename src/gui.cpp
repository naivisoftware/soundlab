#include <gui.h>
#include <settings.h>
#include <napofsplinecomponent.h>
#include <napethercamera.h>
#include <napofsplinemodulationcomponent.h>
#include <napoftracecomponent.h>
#include <napetherservice.h>
#include <settingserializer.h>

// Sets up the gui using the objects found in ofapp
void Gui::Setup()
{
	// Default gui setip
	ofxGuiSetFont(gGetAppSetting<std::string>("GuiFont", "Arial"), gGetAppSetting<int>("GuiFontSize", 8));

	// Populate parameters for spline modulation
	mColorParameters.setName("Color");
	mColorParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFSplineColorComponent>()));

	mXformParameters.setName("Xform");
	mXformParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFTransform>()));

	mRotateParameters.setName("AnimateRotation");
	mRotateParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFRotateComponent>()));

	mScaleParameters.setName("AnimateScale");
	mScaleParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFScaleComponent>()));

	mTraceParameters.setName("Tracer");
	mTraceParameters.addAttribute(mApp.getLaser()->getComponent<nap::EtherDreamCamera>()->mTraceMode);
	mTraceParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFTraceComponent>()));

	mLFOParameters.setName("LFO");
	mLFOParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFSplineLFOModulationComponent>()));

	mSelectionParameters.setName("Selection");
	mSelectionParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFSplineSelectionComponent>()));

	mFileParameters.setName("Spline File");
	mFileParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFSplineFromFileComponent>()));

	// Add parameters to gui
	mSplineGui.setup("spline");
	mSplineGui.add(mColorParameters.getGroup());
	mSplineGui.add(mXformParameters.getGroup());
	mSplineGui.add(mRotateParameters.getGroup());
	mSplineGui.add(mScaleParameters.getGroup());
	mSplineGui.add(mTraceParameters.getGroup());
	mSplineGui.add(mLFOParameters.getGroup());
	mSplineGui.add(mSelectionParameters.getGroup());
	mSplineGui.add(mFileParameters.getGroup());
	mSplineGui.minimizeAll();
	mGuis.emplace_back(&mSplineGui);

	//////////////////////////////////////////////////////////////////////////

	// Populate parameters for laser service
	mLaserServiceParameters.setName("LaserService");
	mLaserServiceParameters.addObject(*(mApp.getLaserService()));

	// Setup laser cam
	mLaserCamParameters.setName("LaserCamera");
	mLaserCamParameters.addAttribute(mApp.getLaser()->getComponent<nap::EtherDreamCamera>()->mFrustrumWidth);
	mLaserCamParameters.addAttribute(mApp.getLaser()->getComponent<nap::EtherDreamCamera>()->mFrustrumHeight);

	// Setup laser
	mLaserGui.setup("laser");
	mLaserGui.add(mLaserCamParameters.getGroup());
	mLaserGui.add(mLaserServiceParameters.getGroup());
	mLaserGui.minimizeAll();
	mGuis.emplace_back(&mLaserGui);

	//////////////////////////////////////////////////////////////////////////

	mSessionParameters.setName("Session");
	mSessionParameters.addObject(*mApp.getSession());
	
	mSessionGui.setup("session");
	mSessionGui.add(mSessionParameters.getGroup());

	mLoad.setup("Load");
	mLoad.addListener(this, &Gui::loadClicked);
	mSave.setup("Save");
	mSessionGui.add(&mLoad);
	mSave.addListener(this, &Gui::saveClicked);
	mSessionGui.add(&mSave);
	mSessionGui.minimizeAll();
	
	mGuis.emplace_back(&mSessionGui);

	//////////////////////////////////////////////////////////////////////////

	// Setup audio gui
	for (auto i = 0; i < mApp.getAudioComposition()->getPlayerCount(); ++i)
	{
		audioGuis.emplace_back(std::make_unique<ofxPanel>());
		mApp.getAudioComposition()->setupGuiForPlayer(*audioGuis.back(), i);
		audioGuis.back()->setName("audio player_" + std::to_string(i));
		mGuis.emplace_back(audioGuis.back().get());
	}
}


/**
@brief Draws all the gui components
**/
void Gui::Draw()
{
	// Draw Gui
	ofEnableAlphaBlending();

	// Draw all guis
	for (auto& gui : mGuis)
		gui->draw();
}


/**
@brief Positions all the gui elements
**/
void Gui::Position(int screenWidth, int screenHeight)
{
	int spacing = gGetAppSetting<int>("GuiSpacing", 25);

	// Position spline gui
	ofPoint current_point(10, 10);
	mSplineGui.setPosition(current_point);
	current_point.x += mSplineGui.getWidth() + spacing;

	// Position laser gui
	mLaserGui.setPosition(current_point);
	current_point.x += mLaserGui.getWidth() + spacing;

	// Position session
	mSessionGui.setPosition(current_point);

	// Position audio gui 1
	current_point.x = screenWidth - (audioGuis[0]->getWidth() * 2) - (spacing * 2);
	audioGuis[0]->setPosition(current_point);

	// Position audio gui 2
	current_point.x += (audioGuis[1]->getWidth() + spacing);
	audioGuis[1]->setPosition(current_point);
}


/**
@brief Loads all the gui settings
**/
void Gui::loadClicked()
{
	static std::string sCurrentDir = ofFilePath::getAbsolutePath(ofFilePath::getCurrentExeDir(), false);
	static std::string sExtension = "xml";

	ofFileDialogResult result = ofSystemLoadDialog("Load Settings", false, sCurrentDir);
	if (!result.bSuccess)
		return;

	// Check what type it is
	ofFile file(result.filePath);

	SettingSerializer serializer;
	serializer.loadSettings(file.getEnclosingDirectory(), *this);
}


/**
@brief Saves all the gui settings
**/
void Gui::saveClicked()
{
	ofFileDialogResult result = ofSystemSaveDialog("Settings", "Name");
	if (!result.bSuccess)
		return;

	ofFile file(result.getPath());
	if (file.exists())
	{
		nap::Logger::warn("file already exists: %s", result.getPath().c_str());
		return;
	}

	SettingSerializer serializer;
	serializer.saveSettings(file.getEnclosingDirectory(), result.getName(), *this);
}
