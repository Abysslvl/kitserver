// setupgui.h

#ifndef _JUCE_SETUPGUI
#define _JUCE_SETUPGUI

#include <windows.h>

#define WIN_WIDTH 535 
//#define WIN_HEIGHT 210
#define WIN_HEIGHT 142

extern HWND g_exeListControl;              // displays list of executable files
extern HWND g_exeInfoControl;              // displays info about current executable file
extern HWND g_setListControl;              // displays list of setting exe files
extern HWND g_setInfoControl;              // displays info about current setting exe file

extern HWND g_installButtonControl;        // restore settings button
extern HWND g_removeButtonControl;         // save settings button
extern HWND g_helpButtonControl;           // help button

extern HWND g_statusTextControl;           // displays status messages

// functions
bool BuildControls(HWND parent);

#endif
