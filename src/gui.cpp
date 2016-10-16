#include <gui.h>
#include <settings.h>
#include <napofsplinecomponent.h>
#include <napethercamera.h>
#include <napofsplinemodulationcomponent.h>
#include <napoftracecomponent.h>
#include <napetherservice.h>
#include <settingserializer.h>
#include <presetcomponent.h>

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
	nap::OFSplineSelectionComponent* spline_selection_comp = mApp.getSpline()->getComponent<nap::OFSplineSelectionComponent>();
	spline_selection_comp->mSplineUpdated.connect(mSplineUpdated);
	mSelectionParameters.addObject(*spline_selection_comp);

	nap::OFSplineFromFileComponent* spline_file_comp = mApp.getSpline()->getComponent<nap::OFSplineFromFileComponent>();
	mFileParameters.setName("Spline File");
	spline_file_comp->mSplineUpdated.connect(mSplineUpdated);
	mFileParameters.addObject(*spline_file_comp);

	mTagParameters.setName("Tag");
	mTagParameters.addObject(*mApp.getSpline());

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
	mSplineGui.add(mTagParameters.getGroup());
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

	// Setup session parameters
	mSessionParameters.setName("Session");
	mSessionParameters.addObject(*mApp.getSession());

	mPresetParameters.setName("PresetSelection");
	mPresetParameters.addObject(*mApp.getSession()->getComponent<nap::PresetComponent>());

	mPresetAutomationParameters.setName("PresetAutomation");
	mPresetAutomationParameters.addObject(*mApp.getSession()->getComponent<nap::PresetSwitchComponent>());

	// Add session parameters to the ui
	mSessionGui.setup("session");
	mSessionGui.add(mSessionParameters.getGroup());

	// Add load / save
	mLoad.setup("Load");
	mLoad.addListener(this, &Gui::loadClicked);
	mSessionGui.add(&mLoad);

	mSave.setup("Save");
	mSave.addListener(this, &Gui::saveClicked);
	mSessionGui.add(&mSave);

	mSaveAs.setup("SaveAs");
	mSaveAs.addListener(this, &Gui::saveAsClicked);
	mSessionGui.add(&mSaveAs);

	mSessionGui.add(mPresetParameters.getGroup());
	mSessionGui.add(mPresetAutomationParameters.getGroup());

	mSessionGui.minimizeAll();

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

	// Draw session gui (not serializable)
	mSessionGui.draw();
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

	ofFileDialogResult result = ofSystemLoadDialog("Load Settings", true, sCurrentDir);
	if (!result.bSuccess)
		return;

	// Check what type it is
	std::string preset_name = result.fileName;

	nap::PresetComponent* preset_comp = mApp.getSession()->getComponent<nap::PresetComponent>();
	assert(preset_comp != nullptr);
	preset_comp->setPreset(preset_name);
}


/**
@brief Saves all the gui settings
**/
void Gui::saveAsClicked()
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

	nap::PresetComponent* preset_comp = mApp.getSession()->getComponent<nap::PresetComponent>();
	assert(preset_comp != nullptr);
	
	// Reload the presets
	preset_comp->loadPresets();
}



void Gui::saveClicked()
{
	nap::PresetComponent* preset_comp = mApp.getSession()->getComponent<nap::PresetComponent>();
	assert(preset_comp != nullptr);
	nap::Preset* current_preset = preset_comp->getCurrentPreset();
	if (current_preset == nullptr)
	{
		nap::Logger::info("there's currently no active preset!");
		return;
	}

	SettingSerializer serializer;
	serializer.saveSettings("saves", current_preset->mPresetName, *this);
	
	// Load all presets
	preset_comp->loadPresets();
}


// Reacts to the update modes of the 2 spline generation components
// This is a bit of a hack but allows the one to disable the other, ensuring correct serialization / deserialization
void Gui::splineUpdated(const nap::Object& obj)
{
	nap::OFSplineSelectionComponent* spline_selection_comp = mApp.getSpline()->getComponent<nap::OFSplineSelectionComponent>();
	assert(spline_selection_comp != nullptr);
	
	nap::OFSplineFromFileComponent* spline_file_comp = mApp.getSpline()->getComponent<nap::OFSplineFromFileComponent>();
	assert(spline_file_comp != nullptr);

	if (&obj == spline_selection_comp)
	{
		spline_file_comp->mAutoUpdate.setValue(false);
		return;
	}

	spline_selection_comp->mAutoUpdate.setValue(false);
}
