#include "GHOST_SystemAndroid.h"
#include <android/native_window_jni.h>
#include <jni.h>

GHOST_SystemAndroid::GHOST_SystemAndroid() : m_window(NULL) {}
GHOST_SystemAndroid::~GHOST_SystemAndroid() {}

bool GHOST_SystemAndroid::processEvents(bool) { return false; }
int GHOST_SystemAndroid::toggleConsole(int) { return 0; }

GHOST_TUns8 GHOST_SystemAndroid::getNumDisplays() const { return 1; }

void GHOST_SystemAndroid::getMainDisplayDimensions(
        GHOST_TUns32 &width, GHOST_TUns32 &height) const
{ width = 0; height = 0; }

void GHOST_SystemAndroid::getAllDisplayDimensions(
        GHOST_TUns32 &width, GHOST_TUns32 &height) const
{ width = 0; height = 0; }

GHOST_IWindow *GHOST_SystemAndroid::createWindow(
        const STR_String &,
        GHOST_TInt32, GHOST_TInt32,
        GHOST_TUns32 width, GHOST_TUns32 height,
        GHOST_TWindowState,
        GHOST_TDrawingContextType,
        GHOST_GLSettings,
        const bool,
        const GHOST_TEmbedderWindowID)
{
	/* Android windows are created via JNI, not this method */
	(void)width; (void)height;
	return NULL;
}

GHOST_TSuccess GHOST_SystemAndroid::getCursorPosition(GHOST_TInt32 &x, GHOST_TInt32 &y) const
{ x = 0; y = 0; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::setCursorPosition(GHOST_TInt32 x, GHOST_TInt32 y)
{ (void)x; (void)y; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::getCursorGrabState(bool &grab, bool &hide) const
{ grab = false; hide = true; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::setCursorGrabState(bool grab, bool hide)
{ (void)grab; (void)hide; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::getCursorVisibility(bool &visible) const
{ visible = true; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::setCursorVisibility(bool visible)
{ (void)visible; return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::getModifierKeys(GHOST_ModifierKeys &keys) const
{ keys.clear(); return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::getButtons(GHOST_Buttons &buttons) const
{ buttons.clear(); return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::setCursorShape(GHOST_TStandardCursor) { return GHOST_kSuccess; }
GHOST_TSuccess GHOST_SystemAndroid::hasCursorShape(GHOST_TStandardCursor) { return GHOST_kFailure; }
GHOST_TSuccess GHOST_SystemAndroid::setCustomCursorShape(
        GHOST_TUns8 *, GHOST_TUns8 *, int, int, int, int, int, int)
{ return GHOST_kSuccess; }

GHOST_TSuccess GHOST_SystemAndroid::getWindowBounds(
        const GHOST_IWindow *, GHOST_Rect &bounds) const
{ return getClientBounds(NULL, bounds); }

GHOST_TSuccess GHOST_SystemAndroid::getClientBounds(
        const GHOST_IWindow *, GHOST_Rect &bounds) const
{
	if (m_window) { bounds = m_window->getClientBounds(); return GHOST_kSuccess; }
	return GHOST_kFailure;
}

GHOST_TUns8 *GHOST_SystemAndroid::getClipboard(bool selection) const
{ (void)selection; return NULL; }

void GHOST_SystemAndroid::putClipboard(GHOST_TInt8 *buffer, bool selection) const
{ (void)buffer; (void)selection; }

/* JNI C API for Android */
static GHOST_SystemAndroid *g_sys = NULL;

extern "C" int GHOST_Android_init(
        JNIEnv *env, jobject surface, int w, int h)
{
	if (!g_sys) g_sys = new GHOST_SystemAndroid();
	ANativeWindow *win = ANativeWindow_fromSurface(env, surface);
	if (!win) return -1;
	GHOST_WindowAndroid *gh_win = new GHOST_WindowAndroid(win, w, h, false, false, 0);
	if (!gh_win->getValid()) { delete gh_win; return -2; }
	g_sys->m_window = gh_win;
	return 0;
}

extern "C" int GHOST_Android_setSurface(
        JNIEnv *env, jobject surface, int w, int h)
{
	if (!g_sys || !g_sys->m_window) return -1;
	ANativeWindow *win = ANativeWindow_fromSurface(env, surface);
	if (!win) return -1;
	g_sys->m_window->setNativeWindow(win, w, h);
	return 0;
}

extern "C" int GHOST_Android_touchEvent(int action, float x, float y)
{
	if (!g_sys) return -1;
	return 0;
}

extern "C" int GHOST_Android_keyEvent(int down, int keyCode, int metaState)
{
	if (!g_sys) return -1;
	return 0;
}
