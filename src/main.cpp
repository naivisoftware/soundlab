#include <ofMain.h>
#include <ofApp.h>
#include <nap/logger.h>
#include <Utils/nofUtils.h>

//========================================================================
int main( )
{
	ofGLWindowSettings window_settings;
	window_settings.glVersionMajor = 3;
	window_settings.glVersionMinor = 3;
	window_settings.windowMode = OF_WINDOW;

	window_settings.width = 1024;
	window_settings.height = 1024;

	// Create window
	shared_ptr<ofAppBaseWindow> main_window = ofCreateWindow(window_settings);
	if (main_window == nullptr)
	{
		nap::Logger::warn("unable to create main window using provided gl settings");
		return -1;
	}

	// Set mip map generation to be fast
	glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);

	// Disable arb texture rendering (normalized uv_coordinates)
	ofDisableArbTex();

	glEnable(GL_LINE_SMOOTH);

	// Set max framerate
	ofSetFrameRate(60);

	// Clear background
	ofBackground(gDefaultBackColor);

	// Enable alpha blending
	ofEnableAlphaBlending();

	// Create app
	std::shared_ptr<ofApp> main_app = std::make_shared<ofApp>();

	// Run app
	ofRunApp(main_window, main_app);

	// Run main loop
	ofRunMainLoop();
}
