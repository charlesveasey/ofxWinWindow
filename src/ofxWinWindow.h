#pragma once

#include "ofConstants.h"
#include "ofAppBaseWindow.h"
#include "ofEvents.h"
#include "ofTypes.h"

class ofBaseApp;

class ofxWinWindow : public ofAppBaseWindow {

public:

	ofxWinWindow();
	~ofxWinWindow(){}

	void setupOpenGL(int w, int h, int screenMode);
		
	void runAppViaInfiniteLoop(ofBaseApp * appPtr);

	void hideCursor();
	void showCursor();
	
	void setFullscreen(bool fullScreen);
	void toggleFullscreen();

	void setWindowTitle(string title);
	void setWindowPosition(int x, int y);
	void setWindowShape(int w, int h);

	ofPoint		getWindowPosition();
	ofPoint		getWindowSize();
	
	int			getWidth();
	int			getHeight();	
	
	int			getWindowMode();

	void		enableSetupScreen();
	void		disableSetupScreen();

	void		setVerticalSync(bool enabled);
	
	void		hideBorder();
	void		showBorder();

	void keepWindowOnTop(bool val);

private:
	static void display(void);
};

