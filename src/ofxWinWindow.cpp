#include "ofxWinWindow.h"
#include "ofBaseApp.h"
#include "ofEvents.h"
#include "ofUtils.h"
#include "ofGraphics.h"
#include "ofAppRunner.h"
#include "ofConstants.h"
#include "ofGLProgrammableRenderer.h"
#include <windows.h>
#include <process.h>
#include <tchar.h>
#include <strsafe.h>

// OF
static int			offsetX;
static int			offsetY;
static int			windowMode;
static bool			bNewScreenMode;
static int			buttonInUse;
static bool			buttonPressed;
static bool			bEnableSetupScreen;
static bool			bDoubleBuffered; 
static int			windowW;
static int			windowH;
static int          nFramesSinceWindowResized;
static ofBaseApp *		ofAppPtr;
void ofGLReadyCallback();

// WIN
static HINSTANCE hInstance;
static const wchar_t CLASS_NAME[]  = L"Sample Window Class";
static HWND hWnd;
static HDC   hDC; 
static HGLRC hRC; 

static LONG WINAPI WndProc (HWND, UINT, WPARAM, LPARAM); 
static BOOL bSetupPixelFormat(HDC);
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//----------------------------------------------------------
// Constructor
//----------------------------------------------------------
ofxWinWindow::ofxWinWindow(){
	windowMode			= OF_WINDOW;
	bNewScreenMode		= true;
	nFramesSinceWindowResized = 0;
	buttonInUse			= 0;
	buttonPressed		= false;
	bEnableSetupScreen	= true;
}

//----------------------------------------------------------
// Create window
//----------------------------------------------------------
void ofxWinWindow::setupOpenGL(int w, int h, int screenMode){

	// create window class
	WNDCLASS wc = { };
	hInstance = GetModuleHandle(NULL);

    wc.style         = 0; 
    wc.lpfnWndProc   = (WNDPROC)WndProc; 
    wc.cbClsExtra    = 0; 
    wc.cbWndExtra    = 0; 
    wc.hInstance     = hInstance; 
    wc.hIcon         = LoadIcon (hInstance, CLASS_NAME); 
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW); 
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); 
    wc.lpszMenuName  = CLASS_NAME; 
    wc.lpszClassName = CLASS_NAME; 

	RegisterClass(&wc);

    // create window
	hWnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"My Window",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
		0, 0, w+16, h+38,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hWnd == NULL)
		std:: cout << "could not create handle" << endl;

	// show window
    ShowWindow(hWnd, SW_SHOWNORMAL);

	// update window
	UpdateWindow(hWnd);

	// get difference between window and drawing area
	RECT rc = { 0, 0, w, h } ;
	GetClientRect(hWnd, &rc);

	// set screen size vars
	offsetX = w - rc.right;
	offsetY = h - rc.bottom;

	windowW = w;
	windowH = h;
}

//----------------------------------------------------------
// Setup pixel format
//----------------------------------------------------------
static BOOL bSetupPixelFormat(HDC hDC) { 
    PIXELFORMATDESCRIPTOR pfd, *ppfd; 
    int pixelformat; 
 
    ppfd = &pfd; 
 
    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR); 
    ppfd->nVersion = 1; 
    ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |  
                        PFD_DOUBLEBUFFER; 
    ppfd->dwLayerMask = PFD_MAIN_PLANE; 
    ppfd->iPixelType = PFD_TYPE_COLORINDEX; 
    ppfd->cColorBits = 8; 
    ppfd->cDepthBits = 16; 
    ppfd->cAccumBits = 0; 
    ppfd->cStencilBits = 0; 
 
    if ( (pixelformat = ChoosePixelFormat(hDC, ppfd)) == 0 ) { 
		MessageBox(NULL, TEXT("ChoosePixelFormat failed"),  TEXT("Error"), MB_OK); 
        return FALSE; 
    } 
 
    if (SetPixelFormat(hDC, pixelformat, ppfd) == FALSE) { 
        MessageBox(NULL, TEXT("SetPixelFormat failed"), TEXT("Error"), MB_OK); 
        return FALSE; 
    } 
 
    return TRUE; 
} 

//----------------------------------------------------------
// Setup gl rendering context, called from winProc 
//----------------------------------------------------------
void wSetupContext(HWND &hwnd){
	RECT rect;	
    hDC = GetDC(hwnd); 
    if (!bSetupPixelFormat(hDC)) 
        PostQuitMessage (0); 
    hRC = wglCreateContext(hDC); 
    wglMakeCurrent(hDC, hRC); 
    GetClientRect(hwnd, &rect);
	ofGLReadyCallback();
}

//----------------------------------------------------------
// Rendering loop
//----------------------------------------------------------
void ofxWinWindow::runAppViaInfiniteLoop(ofBaseApp * appPtr){
	ofAppPtr = appPtr;

	// setup
	ofNotifySetup();
	
	// event -> update -> draw loop
	MSG msg;
    while (1) { 
        while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) { 
            if (GetMessage(&msg, NULL, 0, 0)) { 
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            } 
        } 
		ofNotifyUpdate();
		display();	
    } 
}

//----------------------------------------------------------
// Main draw function
//----------------------------------------------------------
void ofxWinWindow::display(void){

	ofPtr<ofGLProgrammableRenderer> renderer = ofGetGLProgrammableRenderer();
	if(renderer){
		renderer->startRender();
	}

	// set viewport, clear the screen
	ofViewport();
	
	float * bgPtr = ofBgColorPtr();
	ofClear(bgPtr[0]*255,bgPtr[1]*255,bgPtr[2]*255, bgPtr[3]*255);

	// screen adjustments
	if (bEnableSetupScreen)
		ofSetupScreen();

	// call draw()
	ofNotifyDraw();

	// swap
	SwapBuffers(hDC);

	if(renderer){
		renderer->finishRender();
	}
}

static bool shiftK = false;
static bool altK = false;
static bool ctrlK = false;

static int char2OFKey(TCHAR ch) {
	int key = ch;
	
	if (altK) {
		cout << ch << endl;

	}
	// letters
	else if (ch >= 65 && ch <= 90) {
		if (!shiftK)
			key = ch + 32;
	}
	else {
		switch(ch) {
			//numbers
			case 48: if (shiftK) key = 41; break;
			case 49: if (shiftK) key = 33; break;
			case 50: if (shiftK) key = 64; break;
			case 51: if (shiftK) key = 35; break;
			case 52: if (shiftK) key = 36; break;
			case 53: if (shiftK) key = 37; break;
			case 54: if (shiftK) key = 94; break;
			case 55: if (shiftK) key = 38; break;
			case 56: if (shiftK) key = 42; break;
			case 57: if (shiftK) key = 40; break;

			default:
				key = -1;
		}
	}

	return key;
}


static int wParam2OfKey(WPARAM wParam, LPARAM lParam) {

	int key = -1;
	TCHAR ch = TCHAR(wParam);

	switch(wParam) {
		case VK_F1:			key = OF_KEY_F1;		break;
		case VK_F2:			key = OF_KEY_F2;		break;
		case VK_F3:			key = OF_KEY_F3;		break;
		case VK_F4:			key = OF_KEY_F4;		break;
		case VK_F5:			key = OF_KEY_F5;		break;
		case VK_F6:			key = OF_KEY_F6;		break;
		case VK_F7:			key = OF_KEY_F7;		break;
		case VK_F8:			key = OF_KEY_F8;		break;
		case VK_F9:			key = OF_KEY_F9;		break;
		case VK_F10:		key = OF_KEY_F10;		break;
		case VK_F11:		key = OF_KEY_F11;		break;
		case VK_F12:		key = OF_KEY_F12;		break;
		
		case VK_LEFT:		key = OF_KEY_LEFT;		break;
		case VK_RIGHT:		key = OF_KEY_RIGHT;		break;
		case VK_UP:			key = OF_KEY_UP;		break;
		case VK_DOWN:		key = OF_KEY_DOWN;		break;

		case VK_DELETE:		key = OF_KEY_DEL;		break;
		case VK_RETURN:		key = OF_KEY_RETURN;	break;
		case VK_ESCAPE:		key = OF_KEY_ESC;		break;
		case VK_TAB:		key = OF_KEY_TAB;		break;
		case VK_HOME:		key = OF_KEY_HOME;		break;
		case VK_END:		key = OF_KEY_END;		break;
		case VK_INSERT:		key = OF_KEY_INSERT;	break;
		
		case VK_CONTROL:  
			if( lParam & (1 << 24) )  
				key = OF_KEY_RIGHT_CONTROL;
			else
				key = OF_KEY_LEFT_CONTROL;
			break;
		
		case VK_SHIFT: 
			if( (((unsigned short)GetKeyState( VK_RSHIFT )) >> 15) != 1 )  
				key = OF_KEY_RIGHT_SHIFT;
			if( (((unsigned short)GetKeyState( VK_LSHIFT )) >> 15) != 1 ) 
				key = OF_KEY_LEFT_SHIFT;
			break;

		default:
			if (altK || ctrlK)
				key = char2OFKey(ch);
			break;
	}

	return key;
}

//----------------------------------------------------------
// WinProc callback
//----------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	RECT rect; 
    PAINTSTRUCT  ps; 
	POINT p;
	TCHAR ch;
	int key = -1;

	switch(msg){

    case WM_CREATE: 
		wSetupContext(hwnd);
		ofNotifyWindowEntry(0);
        break; 

    case WM_SIZE: 
		GetWindowRect(hwnd, &rect);
		ofNotifyWindowResized(rect.right, rect.bottom);
		GetClientRect(hwnd, &rect);
		windowW = rect.right;
		windowH = rect.bottom;
        break; 

	case WM_LBUTTONDOWN:
		buttonPressed = true;
		buttonInUse = OF_MOUSE_BUTTON_LEFT;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		ofNotifyMousePressed(p.x, p.y, buttonInUse);
		break;

	case WM_RBUTTONDOWN:
		buttonPressed = true;
		buttonInUse = OF_MOUSE_BUTTON_RIGHT;
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		ofNotifyMousePressed(p.x, p.y, buttonInUse);
		break;

	case WM_MBUTTONDOWN:
		buttonPressed = true;
		buttonInUse = OF_MOUSE_BUTTON_MIDDLE;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		ofNotifyMousePressed(p.x, p.y, buttonInUse);
		break;

	case WM_LBUTTONUP:
		buttonPressed = false;
		buttonInUse = OF_MOUSE_BUTTON_LEFT;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		ofNotifyMouseReleased(p.x, p.y, buttonInUse);
		break;

	case WM_RBUTTONUP:
		buttonPressed = false;
		buttonInUse = OF_MOUSE_BUTTON_RIGHT;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		ofNotifyMouseReleased(p.x, p.y, buttonInUse);
		break;

	case WM_MBUTTONUP:
		buttonPressed = false;
		buttonInUse = OF_MOUSE_BUTTON_MIDDLE;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		ofNotifyMouseReleased(p.x, p.y, buttonInUse);
		break;

	case WM_MOUSEMOVE:
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		ofNotifyMouseMoved(p.x, p.y);
		if (buttonPressed)
			ofNotifyMouseDragged(p.x, p.y, buttonInUse);
		break;

	case WM_CHAR: 
		ofNotifyKeyPressed((TCHAR) wParam);
		break;

	case WM_KEYDOWN: 
		if (wParam == VK_SHIFT)
			shiftK = true;
		else if (wParam == VK_CONTROL)
			ctrlK = true;
		key = wParam2OfKey(wParam, lParam);
		if (key > -1)
			ofNotifyKeyPressed(key);
		break;

	case WM_SYSKEYDOWN:
		// alt  
        if( wParam == VK_MENU ) {  
			altK = true;          
			if( lParam & (1 << 24) )
				ofNotifyKeyPressed(OF_KEY_RIGHT_ALT);
            else 
				ofNotifyKeyPressed(OF_KEY_LEFT_ALT);
        } 
		else {
			key = wParam2OfKey(wParam, lParam);
		}
		break;

	case WM_KEYUP: 
		if (wParam == VK_SHIFT)
			shiftK = true;
		else if (wParam == VK_CONTROL)
			ctrlK = false;
		key = wParam2OfKey(wParam, lParam);
		if (key > -1)
			ofNotifyKeyReleased(key);
		break;

	case WM_SYSKEYUP:
		// alt  
        if( wParam == VK_MENU ) {  
			altK = false;          
			if( lParam & (1 << 24) )
				ofNotifyKeyReleased(OF_KEY_RIGHT_ALT);
            else 
				ofNotifyKeyReleased(OF_KEY_LEFT_ALT);
        } 
		else {
			key = wParam2OfKey(wParam, lParam);
		}
		break;

    case WM_PAINT: 
		BeginPaint(hWnd, &ps); 
		EndPaint(hWnd, &ps); 
        break; 
	  
	  case WM_DROPFILES:
         break;

      case WM_CLOSE:
        OF_EXIT_APP(0);
		if (hRC) 
			wglDeleteContext(hRC); 
		if (hDC) 
            ReleaseDC(hWnd, hDC); 
        hRC = 0; 
        hDC = 0; 
        DestroyWindow (hWnd); 
      break;

      case WM_DESTROY: 
         OF_EXIT_APP(0);
		if (hRC) 
			wglDeleteContext(hRC); 
		if (hDC) 
            ReleaseDC(hWnd, hDC); 
        PostQuitMessage (0);
         break;

	  default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
      break;
    }

    return 0;
}

//------------------------------------------------------------
void ofxWinWindow::setWindowTitle(string title){
	std::wstring stemp = std::wstring(title.begin(), title.end());
	LPCWSTR sw = stemp.c_str();
	SetWindowTextW(hWnd, sw);
}

//------------------------------------------------------------
ofPoint ofxWinWindow::getWindowSize(){
	RECT rect;
	GetClientRect(hWnd, &rect);
	return ofPoint(rect.right-rect.left,rect.bottom-rect.top,0);
}

//------------------------------------------------------------
ofPoint ofxWinWindow::getWindowPosition(){
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return  ofPoint(rect.left,rect.top,0);
}

//------------------------------------------------------------
int ofxWinWindow::getWidth(){
	RECT rect;
	GetClientRect(hWnd, &rect);
	return rect.right-rect.left;
}

//------------------------------------------------------------
int ofxWinWindow::getHeight(){
	RECT rect;
	GetClientRect(hWnd, &rect);
	return rect.bottom-rect.top;
}

//------------------------------------------------------------
void ofxWinWindow::setWindowPosition(int x, int y){
	RECT rect;
	GetWindowRect(hWnd, &rect);
	SetWindowPos(hWnd, NULL, x, y, rect.right-rect.left, rect.bottom-rect.top, NULL);
}

//------------------------------------------------------------
void ofxWinWindow::setWindowShape(int w, int h){
	RECT rect;
	GetWindowRect(hWnd, &rect);
	SetWindowPos(hWnd, NULL, rect.left, rect.top, w, h, NULL);
}

//------------------------------------------------------------
void ofxWinWindow::hideCursor(){
	ShowCursor(false);
}

//------------------------------------------------------------
void ofxWinWindow::showCursor(){
	ShowCursor(true);
}

//------------------------------------------------------------
void ofxWinWindow::hideBorder(){
	SetWindowLong(hWnd, GWL_STYLE, 0);
	ShowWindow(hWnd, SW_SHOW);
	//setWindowShape(getWidth(), getHeight());
}

//------------------------------------------------------------
void ofxWinWindow::keepWindowOnTop(bool val){
	RECT rect;
	GetWindowRect(hWnd, &rect);

	if (val)
		SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, NULL);
	else 
		SetWindowPos(hWnd, NULL, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, NULL);
}

//------------------------------------------------------------
void ofxWinWindow::showBorder(){
	SetWindowLong(hWnd, WS_OVERLAPPEDWINDOW, 0);
	ShowWindow(hWnd, SW_NORMAL);
}

//------------------------------------------------------------
int ofxWinWindow::getWindowMode(){
	return windowMode;
}

//------------------------------------------------------------
void ofxWinWindow::toggleFullscreen(){
	if( windowMode == OF_GAME_MODE)return;

	if( windowMode == OF_WINDOW ){
		windowMode = OF_FULLSCREEN;
	}else{
		windowMode = OF_WINDOW;
	}

	bNewScreenMode = true;
}

//------------------------------------------------------------
void ofxWinWindow::setFullscreen(bool fullscreen){
    if( windowMode == OF_GAME_MODE)return;

    if(fullscreen && windowMode != OF_FULLSCREEN){
        bNewScreenMode  = true;
        windowMode      = OF_FULLSCREEN;
    }else if(!fullscreen && windowMode != OF_WINDOW) {
        bNewScreenMode  = true;
        windowMode      = OF_WINDOW;
    }
}

//------------------------------------------------------------
void ofxWinWindow::enableSetupScreen(){
	bEnableSetupScreen = true;
}

//------------------------------------------------------------
void ofxWinWindow::disableSetupScreen(){
	bEnableSetupScreen = false;
}

//------------------------------------------------------------
void ofxWinWindow::setVerticalSync(bool bSync){
	if (bSync) {
		if (WGL_EXT_swap_control) {
			wglSwapIntervalEXT (1);
		}
	} else {
		if (WGL_EXT_swap_control) {
			wglSwapIntervalEXT (0);
		}
	}
}
