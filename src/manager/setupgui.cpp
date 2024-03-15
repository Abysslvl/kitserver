#define UNICODE

#include <windows.h>
#include <stdio.h>
#include "setupgui.h"

extern const wchar_t* _getTransl(char* section, char* key);
#define lang(s) _getTransl("manager",s)

HWND g_exeListControl;              // displays list of executable files
HWND g_exeInfoControl;              // displays info about current executable file
HWND g_setListControl;              // displays list of setting exe files
HWND g_setInfoControl;              // displays info about current setting exe file

HWND g_installButtonControl;        // restore settings button
HWND g_removeButtonControl;         // save settings button
HWND g_helpButtonControl;           // help button

HWND g_statusTextControl;           // displays status messages

/**
 * build all controls
 */
bool BuildControls(HWND parent)
{
	HGDIOBJ hObj;
	DWORD style, xstyle;
	int x, y, spacer;
	int boxW, boxH, statW, statH, borW, borH, butW, butH;

	spacer = 6; 
	x = spacer, y = spacer;
	butH = 20;

	// use default extended style
	xstyle = WS_EX_LEFT;
	style = WS_CHILD | WS_VISIBLE;

	// TOP SECTION

	borW = WIN_WIDTH - spacer*3;
	statW = 120;
	boxW = borW - statW - spacer*3; boxH = 20; statH = 16;
	//borH = spacer*6 + boxH*4 + 20;
	borH = spacer*3 + boxH*2 + 10;

	HWND staticBorderTopControl = CreateWindowEx(
			xstyle, L"Static", L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
			x, y, borW, borH,
			parent, NULL, NULL, NULL);

	x += spacer; y += spacer;

	HWND topLabel = CreateWindowEx(
			xstyle, L"Static", lang("lGameExecutable"), style,
			x, y+4, statW, statH, 
			parent, NULL, NULL, NULL);

	x += statW + spacer;
	boxW = borW - spacer*3 - statW;

	style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL;

	g_exeListControl = CreateWindowEx(
			xstyle, L"ComboBox", L"", style | WS_TABSTOP,
			x, y, boxW, boxH * 6,
			parent, NULL, NULL, NULL);

	x = spacer*2;
	y += boxH + spacer;

	style = WS_CHILD | WS_VISIBLE;


	HWND infoLabel = CreateWindowEx(
			xstyle, L"Static", lang("lCurrentState"), style,
			x, y+4, statW, statH, 
			parent, NULL, NULL, NULL);

	x += statW + spacer;

	g_exeInfoControl = CreateWindowEx(
			xstyle, L"Static", lang("InformationUnavailable"), style,
			x, y+4, boxW, boxH + 10,
			parent, NULL, NULL, NULL);

	x = spacer*2;
	y += boxH + spacer*2 + 10;
	
    /*
	// settings
	
	HWND setListLabel = CreateWindowEx(
			xstyle, L"Static", lang("lSettingsExecutable"), style,
			x, y+4, statW, statH, 
			parent, NULL, NULL, NULL);

	x += statW + spacer;
	boxW = borW - spacer*3 - statW;

	style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL;

	g_setListControl = CreateWindowEx(
			xstyle, L"ComboBox", L"", style | WS_TABSTOP,
			x, y, boxW, boxH * 6,
			parent, NULL, NULL, NULL);

	x = spacer*2;
	y += boxH + spacer;

	style = WS_CHILD | WS_VISIBLE;
	
	HWND infoSetLabel = CreateWindowEx(
			xstyle, L"Static", lang("lCurrentState"), style,
			x, y+4, statW, statH, 
			parent, NULL, NULL, NULL);

	x += statW + spacer;

	g_setInfoControl = CreateWindowEx(
			xstyle, L"Static", lang("InformationUnavailable"), style,
			x, y+4, boxW, boxH + 10,
			parent, NULL, NULL, NULL);

	x = spacer*2;
	y += boxH + spacer*2 + 10;
    */

	// BOTTOM SECTION - buttons

	butW = 60; butH = 24;
	x = WIN_WIDTH - spacer*2 - butW;

	g_removeButtonControl = CreateWindowEx(
			xstyle, L"Button", lang("bRemove"), style | WS_TABSTOP,
			x, y, butW, butH,
			parent, NULL, NULL, NULL);

	butW = 60;
	x -= butW + spacer;

	g_installButtonControl = CreateWindowEx(
			xstyle, L"Button", lang("bInstall"), style | WS_TABSTOP,
			x, y, butW, butH,
			parent, NULL, NULL, NULL);

	butW = 60;
	x -= butW + spacer;

    /*
	g_helpButtonControl = CreateWindowEx(
			xstyle, L"Button", lang("bHelp"), style | WS_TABSTOP,
			x, y, butW, butH,
			parent, NULL, NULL, NULL);

    */
	x = spacer;
	statW = WIN_WIDTH - spacer*5 - 220;

	g_statusTextControl = CreateWindowEx(
			xstyle, L"Static", L"", style,
			x, y+6, statW, statH,
			parent, NULL, NULL, NULL);

	// If any control wasn't created, return false
	if (topLabel == NULL ||
		g_exeListControl == NULL ||
		//osLabel == NULL ||
		//g_osListControl == NULL ||
		infoLabel == NULL ||
		g_exeInfoControl == NULL ||
		g_statusTextControl == NULL ||
		g_installButtonControl == NULL ||
		g_removeButtonControl == NULL)
		return false;

	// Get a handle to the stock font object
	hObj = GetStockObject(DEFAULT_GUI_FONT);

	SendMessage(topLabel, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_exeListControl, WM_SETFONT, (WPARAM)hObj, true);
	//SendMessage(setListLabel, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_setListControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(infoLabel, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_exeInfoControl, WM_SETFONT, (WPARAM)hObj, true);
	//SendMessage(infoSetLabel, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_setInfoControl, WM_SETFONT, (WPARAM)hObj, true);

	SendMessage(g_statusTextControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_installButtonControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_removeButtonControl, WM_SETFONT, (WPARAM)hObj, true);
	SendMessage(g_helpButtonControl, WM_SETFONT, (WPARAM)hObj, true);

	// disable the dropdown list and the buttons by default
	EnableWindow(g_installButtonControl, FALSE);
	EnableWindow(g_removeButtonControl, FALSE);
	EnableWindow(g_exeListControl, FALSE);
	EnableWindow(g_setListControl, FALSE);

	return true;
}
