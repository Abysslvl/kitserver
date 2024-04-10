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

enum
{
    MsgTitle,
    HelpTextTitle,
    ManualPath,
    lGameExecutable,
    lCurrentState,
    InformationUnavailable,
    NoActionRequired,
    NoAction,
    bInstall,
    bRemove,
    ParamSettings,
    ParamGame,
    Err_UnknownExe,
    Err_WrongExeType,
    HelpText,
};

constexpr const wchar_t* managerLabels[] =
{
    L"Kitserver Setup Message",
    L"Kitserver short help",
    L".\\docs\\manual.html",
    L"Game executable:",
    L"Current state:",
    L"Information unavailable",
    L"No action required",
    L"- no action -",
    L"Attach",
    L"Detach",
    L"settings",
    L"game",
    L"======== WRONG FILE! =========\n"
    L"File % s is an unknown EXE - file."
    L"Therefore,"
    L"Kitserver will NOT be attached to it.",
    L"======== WRONG FILE! =========\n"
    L"File %s is a %s EXE instead of a %s EXE-file."
    L"Therefore,"
    L"Kitserver will NOT be attached to it.",
    L"For information on what Kitserver is, what you can do with"
    L"it and how you install / use it correctly, please read the manual.html"
    L"which is located in the in the \"docs\" subfolder",
};

#endif
