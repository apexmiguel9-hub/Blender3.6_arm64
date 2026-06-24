#ifndef __GHOST_WINDOWANDROID_H__
#define __GHOST_WINDOWANDROID_H__

#include "GHOST_Window.h"
#include "GHOST_Rect.h"
#include "STR_String.h"
#include <android/native_window.h>
#include <EGL/egl.h>

class GHOST_WindowAndroid : public GHOST_Window {
public:
	GHOST_WindowAndroid(ANativeWindow *nativeWindow,
	        GHOST_TInt32 width, GHOST_TInt32 height,
	        bool stereoVisual, bool alphaBackground,
	        GHOST_TUns16 numOfAASamples);
	~GHOST_WindowAndroid();

	GHOST_TSuccess setNativeWindow(ANativeWindow *window, int w, int h);
	GHOST_Rect getClientBounds() const { return m_clientBounds; }
	bool getValid() const;
	void setTitle(const STR_String &title);
	void getTitle(STR_String &title) const;
	void getWindowBounds(GHOST_Rect &bounds) const;
	void getClientBounds(GHOST_Rect &bounds) const;
	GHOST_TSuccess setClientWidth(GHOST_TUns32 width);
	GHOST_TSuccess setClientHeight(GHOST_TUns32 height);
	GHOST_TSuccess setClientSize(GHOST_TUns32 width, GHOST_TUns32 height);
	void screenToClient(GHOST_TInt32 inX, GHOST_TInt32 inY,
	        GHOST_TInt32 &outX, GHOST_TInt32 &outY) const;
	void clientToScreen(GHOST_TInt32 inX, GHOST_TInt32 inY,
	        GHOST_TInt32 &outX, GHOST_TInt32 &outY) const;
	GHOST_TSuccess swapBuffers();
	GHOST_TSuccess setSwapInterval(int interval);
	GHOST_TSuccess getSwapInterval(int &intervalOut);

	GHOST_TWindowState getState() const;
	GHOST_TSuccess setState(GHOST_TWindowState state);
	GHOST_TSuccess setOrder(GHOST_TWindowOrder order);
	GHOST_TSuccess invalidate();
	const GHOST_TabletData *GetTabletData();
	GHOST_TSuccess beginFullScreen() const;
	GHOST_TSuccess endFullScreen() const;
	GHOST_Context *newDrawingContext(GHOST_TDrawingContextType type);
	GHOST_TSuccess setWindowCursorVisibility(bool visible);
	GHOST_TSuccess setWindowCursorShape(GHOST_TStandardCursor shape);
	GHOST_TSuccess setWindowCustomCursorShape(
	        GHOST_TUns8 bitmap[16][2], GHOST_TUns8 mask[16][2],
	        GHOST_TUns32 hotX, GHOST_TUns32 hotY);
	GHOST_TSuccess setWindowCustomCursorShape(
	        GHOST_TUns8 *bitmap, GHOST_TUns8 *mask,
	        int sizex, int sizey, int hotX, int hotY,
	        int fgColor, int bgColor);

private:
	ANativeWindow *m_nativeWindow;
	GHOST_Rect m_clientBounds;
	EGLDisplay m_display;
	EGLSurface m_surface;
	EGLContext m_context;
	bool m_valid;
};

#endif
