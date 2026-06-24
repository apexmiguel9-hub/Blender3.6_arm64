#include "GHOST_WindowAndroid.h"
#include <cstdio>

GHOST_WindowAndroid::GHOST_WindowAndroid(
        ANativeWindow *nativeWindow,
        GHOST_TInt32 width, GHOST_TInt32 height,
        bool stereoVisual, bool alphaBackground,
        GHOST_TUns16 numOfAASamples)
    : GHOST_Window(width, height, GHOST_kWindowStateNormal, stereoVisual, false, numOfAASamples),
      m_nativeWindow(nativeWindow),
      m_display(EGL_NO_DISPLAY),
      m_surface(EGL_NO_SURFACE),
      m_context(EGL_NO_CONTEXT),
      m_valid(false)
{
	m_clientBounds.m_r = width;
	m_clientBounds.m_b = height;

	m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (m_display == EGL_NO_DISPLAY) { fprintf(stderr, "eglGetDisplay fail\n"); return; }

	EGLint major, minor;
	if (!eglInitialize(m_display, &major, &minor)) { fprintf(stderr, "eglInit fail\n"); return; }

	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 24, EGL_NONE
	};
	EGLConfig config;
	EGLint numConfigs;
	if (!eglChooseConfig(m_display, attribs, &config, 1, &numConfigs) || numConfigs == 0)
		{ fprintf(stderr, "eglChooseConfig fail\n"); return; }

	m_surface = eglCreateWindowSurface(m_display, config, nativeWindow, NULL);
	if (m_surface == EGL_NO_SURFACE) { fprintf(stderr, "eglCreateWindowSurface fail\n"); return; }

	const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, ctxAttribs);
	if (m_context == EGL_NO_CONTEXT) { fprintf(stderr, "eglCreateContext fail\n"); return; }

	if (eglMakeCurrent(m_display, m_surface, m_surface, m_context))
		m_valid = true;
}

GHOST_WindowAndroid::~GHOST_WindowAndroid()
{
	if (m_display != EGL_NO_DISPLAY) {
		eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (m_context != EGL_NO_CONTEXT) eglDestroyContext(m_display, m_context);
		if (m_surface != EGL_NO_SURFACE) eglDestroySurface(m_display, m_surface);
		eglTerminate(m_display);
	}
	if (m_nativeWindow) ANativeWindow_release(m_nativeWindow);
}

GHOST_TSuccess GHOST_WindowAndroid::setNativeWindow(ANativeWindow *win, int w, int h)
{
	if (m_nativeWindow) ANativeWindow_release(m_nativeWindow);
	m_nativeWindow = win;
	m_clientBounds.m_r = w;
	m_clientBounds.m_b = h;

	if (m_display != EGL_NO_DISPLAY) {
		EGLConfig config;
		EGLint numConfigs;
		const EGLint attribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
			EGL_DEPTH_SIZE, 24, EGL_NONE
		};
		eglChooseConfig(m_display, attribs, &config, 1, &numConfigs);
		EGLSurface surf = eglCreateWindowSurface(m_display, config, win, NULL);
		if (surf != EGL_NO_SURFACE) {
			if (m_surface != EGL_NO_SURFACE) eglDestroySurface(m_display, m_surface);
			m_surface = surf;
			eglMakeCurrent(m_display, m_surface, m_surface, m_context);
		}
	}
	return GHOST_kSuccess;
}

bool GHOST_WindowAndroid::getValid() const { return m_valid; }
void GHOST_WindowAndroid::setTitle(const STR_String &) {}
void GHOST_WindowAndroid::getTitle(STR_String &title) const { title = "Blender"; }
void GHOST_WindowAndroid::getWindowBounds(GHOST_Rect &b) const { b = m_clientBounds; }
void GHOST_WindowAndroid::getClientBounds(GHOST_Rect &b) const { b = m_clientBounds; }
GHOST_TSuccess GHOST_WindowAndroid::setClientWidth(GHOST_TUns32 w)
{ m_clientBounds.m_r = w; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_WindowAndroid::setClientHeight(GHOST_TUns32 h)
{ m_clientBounds.m_b = h; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_WindowAndroid::setClientSize(GHOST_TUns32 w, GHOST_TUns32 h)
{ m_clientBounds.m_r = w; m_clientBounds.m_b = h; return GHOST_kSuccess; }
void GHOST_WindowAndroid::screenToClient(
        GHOST_TInt32 inX, GHOST_TInt32 inY, GHOST_TInt32 &outX, GHOST_TInt32 &outY) const
{ outX = inX; outY = inY; }
void GHOST_WindowAndroid::clientToScreen(
        GHOST_TInt32 inX, GHOST_TInt32 inY, GHOST_TInt32 &outX, GHOST_TInt32 &outY) const
{ outX = inX; outY = inY; }
GHOST_TSuccess GHOST_WindowAndroid::swapBuffers()
{ return eglSwapBuffers(m_display, m_surface) ? GHOST_kSuccess : GHOST_kFailure; }
GHOST_TSuccess GHOST_WindowAndroid::setSwapInterval(int i)
{ return eglSwapInterval(m_display, i) ? GHOST_kSuccess : GHOST_kFailure; }
GHOST_TSuccess GHOST_WindowAndroid::getSwapInterval(int &i)
{ (void)i; return GHOST_kFailure; }

GHOST_TWindowState GHOST_WindowAndroid::getState() const { return GHOST_kWindowStateNormal; }
GHOST_TSuccess GHOST_WindowAndroid::setState(GHOST_TWindowState) { return GHOST_kSuccess; }
GHOST_TSuccess GHOST_WindowAndroid::setOrder(GHOST_TWindowOrder) { return GHOST_kSuccess; }
GHOST_TSuccess GHOST_WindowAndroid::invalidate() { return GHOST_kSuccess; }
const GHOST_TabletData *GHOST_WindowAndroid::GetTabletData() { return NULL; }
GHOST_TSuccess GHOST_WindowAndroid::beginFullScreen() const { return GHOST_kFailure; }
GHOST_TSuccess GHOST_WindowAndroid::endFullScreen() const { return GHOST_kFailure; }
GHOST_Context *GHOST_WindowAndroid::newDrawingContext(GHOST_TDrawingContextType)
{ return NULL; }
GHOST_TSuccess GHOST_WindowAndroid::setWindowCursorVisibility(bool) { return GHOST_kSuccess; }
GHOST_TSuccess GHOST_WindowAndroid::setWindowCursorShape(GHOST_TStandardCursor)
{ return GHOST_kSuccess; }
GHOST_TSuccess GHOST_WindowAndroid::setWindowCustomCursorShape(
        GHOST_TUns8[16][2], GHOST_TUns8[16][2], int, int)
{ return GHOST_kSuccess; }
GHOST_TSuccess GHOST_WindowAndroid::setWindowCustomCursorShape(
        GHOST_TUns8 *, GHOST_TUns8 *, int, int, int, int, int, int)
{ return GHOST_kSuccess; }
