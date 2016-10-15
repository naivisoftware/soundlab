#include <gui.h>
#include <settings.h>
#include <napofsplinecomponent.h>
#include <napethercamera.h>
#include <napofsplinemodulationcomponent.h>
#include <napoftracecomponent.h>
#include <napetherservice.h>

// Sets up the gui using the objects found in ofapp
void Gui::Setup()
{
	// Default gui setip
	ofxGuiSetFont(gGetAppSetting<std::string>("GuiFont", "Arial"), gGetAppSetting<int>("GuiFontSize", 8));

	// Populate parameters for spline modulation
	mColorParameters.setName("Color");
	mColorParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFSplineColorComponent>()));

	mRotateParameters.setName("Rotation");
	mRotateParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFRotateComponent>()));

	mScaleParameters.setName("Scale");
	mScaleParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFScaleComponent>()));
	mScaleParameters.addObject(*(mApp.getSpline()->getComponent<nap::OFTransform>()));

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
	mLaserServiceParameters.addObject(*(mApp.getLaserService()));

	// Setup laser cam
	mLaserCamParameters.setName("LaserCamera");
	mLaserCamParameters.addAttribute(mApp.getLaser()->getComponent<nap::EtherDreamCamera>()->mFrustrumWidth);
	mLaserCamParameters.addAttribute(mApp.getLaser()->getComponent<nap::EtherDreamCamera>()->mFrustrumHeight);

	// Setup laser
	mLaserGui.setup();
	mLaserGui.add(mLaserCamParameters.getGroup());
	mLaserGui.add(mLaserServiceParameters.getGroup());
	mLaserGui.minimizeAll();

	// Setup audio gui
	for (auto i = 0; i < mApp.getAudioComposition()->getPlayerCount(); ++i)
	{
		audioGuis.emplace_back(std::make_unique<ofxPanel>());
		mApp.getAudioComposition()->setupGuiForPlayer(*audioGuis.back(), i);
	}
}


/**
@brief Draws all the guis
**/
void Gui::Draw()
{
	// Draw Gui
	ofEnableAlphaBlending();
	mSplineGui.draw();
	mLaserGui.draw();
	for (auto& audioGui : audioGuis) 
		audioGui->draw();
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

	// Position audio gui 1
	current_point.x = screenWidth - (audioGuis[0]->getWidth() * 2) - (spacing * 2);
	audioGuis[0]->setPosition(current_point);

	// Position audio gui 2
	current_point.x += (audioGuis[1]->getWidth() + spacing);
	audioGuis[1]->setPosition(current_point);
}
