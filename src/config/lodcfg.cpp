/* main */
/* Version 1.0 */

#include <windows.h>
#include <windef.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <commctrl.h>

#define BUFLEN 4096

#include "lodcfgui.h"
#include "lodcfg.h"
#include "detect.h"
#include "imageutil.h"
#include "configs.h"
#include "lodmixer.h"
#include "vstyles.h"

#define UNDEFINED -1
#define WM_APP_EXECHANGE WM_APP+1
#define CFG_FILE L"config.txt"
#define CFG_DEFAULT_FILE L"config-default.txt"

HWND hWnd = NULL;
bool g_buttonClick = true;
BOOL g_isBeingEdited = FALSE;

char* _help = 
"Enabling aspect ratio correction will allow you to play with \n\
round ball and correct proportions on any resolution. You can \n\
either let LOD mixer calculate it automatically at run-time \n\
based on the resolution, or set it manually here.\n\
\n\
Enforcing particular resolution is also possible. If you play in \n\
a windowed mode, any resolution will work. But for full-screen, \n\
only those that are REALLY supported by your videocard will be\n\
in full-screen. Unsupported resolutions will switch back to\n\
windowed mode.\n\
\n\
Enforcing picture quality allows you to choose any of the three \n\
available modes: Low, Medium, High. Be advised though, that if \n\
you do that Kitserver will disable to video adapter test logic in the \n\
game and if your video card really does not support the chosen \n\
mode then the game will likely crash.\n\
\n\
About Free Sides Select:\n\
In competition modes (League/Cup, UEFA Champions League, etc.)\n\
PES 2012 doesn't allow human players to freely select which team\n\
to control. Now you can remove that limitation. So, even for\n\
P1 vs. COM game, or P2 vs. COM - you can freely choose which\n\
team you control with each controller. This also allows to\n\
play defence/goalkeeper in training mode.\n\
\n\
The game LOD (level-of-detail) is adjustable. The sliders indicate \n\
when the switching between the levels occurs. Moving sliders to \n\
the left makes the game engine to switch earlier - as a result you \n\
get less detailed models. You can try that if your PC is \n\
struggling and the framerate is poor. If, however, you've got a \n\
pretty powerful machine and a good videocard, then try moving the \n\
sliders to the right: the game engine will switch the LODs later, \n\
and therefore give you more detail.\n\
\n\
Dont't forget to press the [Save] button!";


LMCONFIG _lmconfig = {
    {DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_ASPECT_RATIO}, 
    {
        DEFAULT_LOD_1,
        DEFAULT_LOD_2,
        DEFAULT_LOD_3,
        DEFAULT_LOD_4,
        DEFAULT_LOD_5,
        DEFAULT_LOD_6,
        DEFAULT_LOD_7,
        DEFAULT_LOD_8,
        DEFAULT_LOD_9,
        DEFAULT_LOD_10,
        DEFAULT_LOD_11,
        DEFAULT_LOD_12,
        DEFAULT_LOD_13,
        DEFAULT_LOD_14,
        DEFAULT_LOD_15,
        DEFAULT_LOD_16,
        DEFAULT_LOD_17,
        DEFAULT_LOD_18,
        DEFAULT_LOD_19,
        DEFAULT_LOD_20,
    },
    DEFAULT_ASPECT_RATIO_CORRECTION_ENABLED,
    DEFAULT_CONTROLLER_CHECK_ENABLED,
    DEFAULT_LODCHECK1,
    DEFAULT_PICTURE_QUALITY,
};
DWORD _cameraAngle = 0;
DWORD _matchTime = 0;
float _speedFactor = 0.0f;

// function prototypes
void lodmixerConfig(char* pName, const void* pValue, DWORD a);
void EnableControls(BOOL flag);
void SaveConfig();
void UpdateControls(LMCONFIG& cfg);
void UpdateConfig(LMCONFIG& cfg);
void ResetLodSwitches(int x);

void ResetLodSwitches(int x)
{
    switch (x)
    {
    case 1:
        SendMessage(g_lodEditControl[0],WM_SETTEXT,0,(LPARAM)"100.0");
        SendMessage(g_lodEditControl[1],WM_SETTEXT,0,(LPARAM)"100.0");
        SendMessage(g_lodEditControl[2],WM_SETTEXT,0,(LPARAM)"0.100");
        SendMessage(g_lodTrackBarControl[0],TBM_SETPOS,TRUE,(LPARAM)getTickValue(100.0));
        SendMessage(g_lodTrackBarControl[1],TBM_SETPOS,TRUE,(LPARAM)getTickValue(100.0));
        SendMessage(g_lodTrackBarControl[2],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.100));
        break;
    case 2:
        SendMessage(g_lodEditControl[3],WM_SETTEXT,0,(LPARAM)"0.090");
        SendMessage(g_lodEditControl[4],WM_SETTEXT,0,(LPARAM)"0.070");
        SendMessage(g_lodEditControl[5],WM_SETTEXT,0,(LPARAM)"0.055");
        SendMessage(g_lodTrackBarControl[3],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.090));
        SendMessage(g_lodTrackBarControl[4],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.070));
        SendMessage(g_lodTrackBarControl[5],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.055));
        break;
    case 3:
        SendMessage(g_lodEditControl[6],WM_SETTEXT,0,(LPARAM)"100.0");
        SendMessage(g_lodEditControl[7],WM_SETTEXT,0,(LPARAM)"0.150");
        SendMessage(g_lodEditControl[8],WM_SETTEXT,0,(LPARAM)"0.100");
        SendMessage(g_lodTrackBarControl[6],TBM_SETPOS,TRUE,(LPARAM)getTickValue(100.0));
        SendMessage(g_lodTrackBarControl[7],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.150));
        SendMessage(g_lodTrackBarControl[8],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.100));
        break;
    case 4:
        SendMessage(g_lodEditControl[9],WM_SETTEXT,0,(LPARAM)"0.430");
        SendMessage(g_lodEditControl[10],WM_SETTEXT,0,(LPARAM)"0.150");
        SendMessage(g_lodEditControl[11],WM_SETTEXT,0,(LPARAM)"0.100");
        SendMessage(g_lodTrackBarControl[9],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.430));
        SendMessage(g_lodTrackBarControl[10],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.150));
        SendMessage(g_lodTrackBarControl[11],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.100));
        break;
    case 5:
        SendMessage(g_lodEditControl[12],WM_SETTEXT,0,(LPARAM)"0.100");
        SendMessage(g_lodTrackBarControl[12],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.100));
        break;
    case 6:
        SendMessage(g_lodEditControl[13],WM_SETTEXT,0,(LPARAM)"0.100");
        SendMessage(g_lodTrackBarControl[13],TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.100));
        break;
    case 7:
        SendMessage(g_lodEditControl[14],WM_SETTEXT,0,(LPARAM)"0.010");
        SendMessage(g_lodEditControl[15],WM_SETTEXT,0,(LPARAM)"0.010");
        SendMessage(g_lodEditControl[16],WM_SETTEXT,0,(LPARAM)"0.010");
        SendMessage(g_lodTrackBarControl[14],
                TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.010));
        SendMessage(g_lodTrackBarControl[15],
                TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.010));
        SendMessage(g_lodTrackBarControl[16],
                TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.010));
        break;
    case 8:
        SendMessage(g_lodEditControl[17],WM_SETTEXT,0,(LPARAM)"100.0");
        SendMessage(g_lodEditControl[18],WM_SETTEXT,0,(LPARAM)"0.010");
        SendMessage(g_lodEditControl[19],WM_SETTEXT,0,(LPARAM)"0.010");
        SendMessage(g_lodTrackBarControl[17],
                TBM_SETPOS,TRUE,(LPARAM)getTickValue(100.0));
        SendMessage(g_lodTrackBarControl[18],
                TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.010));
        SendMessage(g_lodTrackBarControl[19],
                TBM_SETPOS,TRUE,(LPARAM)getTickValue(0.010));
        break;
    }
}

void ResetSpeedSwitch()
{
    SendMessage(g_speedEditControl,WM_SETTEXT,0,(LPARAM)"1.00");
    SendMessage(g_speedTrackBarControl,TBM_SETPOS,TRUE,
            (LPARAM)getTickValue2(1.0));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc1(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // get the tab message from lParam
    LPNMHDR lpnmhdr = (LPNMHDR)lParam;

    switch(uMsg)
    {
        case WM_NOTIFY:
            // if we received the TCN_SELCHANGE message, process it
            // (TCN_SELCHANGE is when the selection changes from
            // one tab item to another)
            if (lpnmhdr->code == TCN_SELCHANGE)
            {
                // get the currently selected tab item
                int iPage = TabCtrl_GetCurSel(hTab);

                // hide and show the appropriate tab view
                // based on which tab item was clicked
                switch (iPage)
                {
                    // Tab1 (item 0) was clicked
                    case 0:
                        ShowLODTab(SW_HIDE);
                        ShowMiscTab(SW_SHOW);
                        break;

                    // Tab2 (item 1) was clicked
                    case 1:
                        ShowMiscTab(SW_HIDE);
                        ShowLODTab(SW_SHOW);
                        break;

                    default:
                        // don't do anything if the tab item isn't 0 or 1
                        break;
                }
            }
            else {
                // forward
                WindowProc(hWnd, WM_COMMAND, wParam, lParam);
            }
            break;
    }
    return WindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			// Exit the application when the window closes
			PostQuitMessage(1);
			return true;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if ((HWND)lParam == g_saveButtonControl)
				{
					// save LOD
					g_buttonClick = true;

                    char exeName[BUFLEN];
                    ZeroMemory(exeName, BUFLEN);
					SaveConfig();
				}
				else if ((HWND)lParam == g_speedCheckBox)
                {
                    bool checked = SendMessage(g_speedCheckBox,BM_GETCHECK,0,0);
                    EnableWindow(g_speedTrackBarControl, checked);
                    EnableWindow(g_speedEditControl, checked);
                    if (!checked) {
                        ResetSpeedSwitch();
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox1)
                {
                    bool checked = SendMessage(g_lodCheckBox1,BM_GETCHECK,0,0);
                    for (int i=0; i<3; i++)
                    {
                        EnableWindow(g_lodTrackBarControl[i], checked);
                        EnableWindow(g_lodEditControl[i], checked);
                    }
                    EnableWindow(g_lodLabel1, checked);
                    EnableWindow(g_lodLabel2, checked);
                    EnableWindow(g_lodLabel3, checked);
                    if (!checked) {
                        ResetLodSwitches(1);
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox2)
                {
                    bool checked = SendMessage(g_lodCheckBox2,BM_GETCHECK,0,0);
                    for (int i=3; i<6; i++)
                    {
                        EnableWindow(g_lodTrackBarControl[i], checked);
                        EnableWindow(g_lodEditControl[i], checked);
                    }
                    EnableWindow(g_lodLabel4, checked);
                    EnableWindow(g_lodLabel5, checked);
                    EnableWindow(g_lodLabel6, checked);
                    if (!checked) {
                        ResetLodSwitches(2);
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox3)
                {
                    bool checked = SendMessage(g_lodCheckBox3,BM_GETCHECK,0,0);
                    for (int i=6; i<9; i++)
                    {
                        EnableWindow(g_lodTrackBarControl[i], checked);
                        EnableWindow(g_lodEditControl[i], checked);
                    }
                    EnableWindow(g_lodLabel7, checked);
                    EnableWindow(g_lodLabel8, checked);
                    EnableWindow(g_lodLabel9, checked);
                    if (!checked) {
                        ResetLodSwitches(3);
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox4)
                {
                    bool checked = SendMessage(g_lodCheckBox4,BM_GETCHECK,0,0);
                    for (int i=9; i<12; i++)
                    {
                        EnableWindow(g_lodTrackBarControl[i], checked);
                        EnableWindow(g_lodEditControl[i], checked);
                    }
                    EnableWindow(g_lodLabel10, checked);
                    EnableWindow(g_lodLabel11, checked);
                    EnableWindow(g_lodLabel12, checked);
                    if (!checked) {
                        ResetLodSwitches(4);
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox5)
                {
                    bool checked = SendMessage(g_lodCheckBox5,BM_GETCHECK,0,0);
                    EnableWindow(g_lodTrackBarControl[12], checked);
                    EnableWindow(g_lodEditControl[12], checked);
                    EnableWindow(g_lodLabel13, checked);
                    if (!checked) {
                        ResetLodSwitches(5);
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox6)
                {
                    bool checked = SendMessage(g_lodCheckBox6,BM_GETCHECK,0,0);
                    EnableWindow(g_lodTrackBarControl[13], checked);
                    EnableWindow(g_lodEditControl[13], checked);
                    EnableWindow(g_lodLabel14, checked);
                    if (!checked) {
                        ResetLodSwitches(6);
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox7)
                {
                    bool checked = SendMessage(g_lodCheckBox7,BM_GETCHECK,0,0);
                    for (int i=14; i<17; i++)
                    {
                        EnableWindow(g_lodTrackBarControl[i], checked);
                        EnableWindow(g_lodEditControl[i], checked);
                    }
                    EnableWindow(g_lodLabel15, checked);
                    EnableWindow(g_lodLabel16, checked);
                    EnableWindow(g_lodLabel17, checked);
                    if (!checked) {
                        ResetLodSwitches(7);
                    }
                }
				else if ((HWND)lParam == g_lodCheckBox8)
                {
                    bool checked = SendMessage(g_lodCheckBox8,BM_GETCHECK,0,0);
                    for (int i=17; i<20; i++)
                    {
                        EnableWindow(g_lodTrackBarControl[i], checked);
                        EnableWindow(g_lodEditControl[i], checked);
                    }
                    EnableWindow(g_lodLabel18, checked);
                    EnableWindow(g_lodLabel19, checked);
                    EnableWindow(g_lodLabel20, checked);
                    if (!checked) {
                        ResetLodSwitches(8);
                    }
                }
				else if ((HWND)lParam == g_arCheckBox)
                {
                    bool checked = SendMessage(g_arCheckBox,BM_GETCHECK,0,0);
                    EnableWindow(g_arRadio1, checked);
                    EnableWindow(g_arRadio2, checked);
                    if (checked) {
                        bool checked1 = SendMessage(g_arRadio2,BM_GETCHECK,0,0);
                        EnableWindow(g_arEditControl, checked1);
                    } else {
                        EnableWindow(g_arEditControl, false);
                    }
                }
				else if ((HWND)lParam == g_pqCheckBox)
                {
                    bool checked = SendMessage(g_pqCheckBox,BM_GETCHECK,0,0);
                    bool chosen(false);
                    for (int i=0; i<3; i++) {
                        EnableWindow(g_pqRadio[i], checked);
                        chosen = chosen || SendMessage(
                            g_pqRadio[i],BM_GETCHECK,0,0);
                    }
                    // check Low, if none chosen
                    if (checked && !chosen) {
                        SendMessage(g_pqRadio[0], BM_SETCHECK, BST_CHECKED, 0);
                    }
                }
				else if ((HWND)lParam == g_arRadio1)
                {
                    EnableWindow(g_arEditControl, false);
                }
				else if ((HWND)lParam == g_arRadio2)
                {
                    EnableWindow(g_arEditControl, true);
                }
				else if ((HWND)lParam == g_resCheckBox)
                {
                    bool checked = SendMessage(g_resCheckBox,BM_GETCHECK,0,0);
                    EnableWindow(g_resWidthControl, checked);
                    EnableWindow(g_resHeightControl, checked);
                }
				else if ((HWND)lParam == g_timeCheckBox)
                {
                    bool checked = SendMessage(g_timeCheckBox,BM_GETCHECK,0,0);
                    EnableWindow(g_timeControl, checked);
                }
				else if ((HWND)lParam == g_defButtonControl)
				{
					// reset defaults 
					g_buttonClick = true;

                    // display small help window
                    MessageBox(hWnd, _help, "Configuration Tool Help", 0);
				}
			}
			else if (HIWORD(wParam) == CBN_EDITUPDATE)
			{
				g_isBeingEdited = TRUE;
			}
			else if (HIWORD(wParam) == CBN_KILLFOCUS && g_isBeingEdited)
			{
				g_isBeingEdited = FALSE;
				HWND control = (HWND)lParam;
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				HWND control = (HWND)lParam;
			}
			break;

		case WM_HSCROLL:
            if (LOWORD(wParam) == TB_THUMBPOSITION || LOWORD(wParam) == TB_THUMBTRACK ||
                    LOWORD(wParam)==TB_LINEUP || LOWORD(wParam)==TB_LINEDOWN ||
                    LOWORD(wParam)==TB_PAGEUP || LOWORD(wParam)==TB_PAGEDOWN)
            {
                if ((HWND)lParam == g_speedTrackBarControl)
                {
                    int val0 = SendMessage(g_speedTrackBarControl,TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.2f",getSwitchValue2(val0));
                    SendMessage(g_speedEditControl,WM_SETTEXT,0,(LPARAM)buf);
                }
                else if ((HWND)lParam == g_lodTrackBarControl[0])
                {
                    int val0 = SendMessage(g_lodTrackBarControl[0],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[0],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val1 = SendMessage(g_lodTrackBarControl[1],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[2],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[1],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[1],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[2],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[2],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[1])
                {
                    int val1 = SendMessage(g_lodTrackBarControl[1],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val1));
                    SendMessage(g_lodEditControl[1],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[0],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[2],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[0],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[0],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[2],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[2],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[2])
                {
                    int val2 = SendMessage(g_lodTrackBarControl[2],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val2));
                    SendMessage(g_lodEditControl[2],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[0],TBM_GETPOS,0,0);
                    int val1 = SendMessage(g_lodTrackBarControl[1],TBM_GETPOS,0,0);
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[0],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[0],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[1],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[1],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                }
                if ((HWND)lParam == g_lodTrackBarControl[3])
                {
                    int val0 = SendMessage(g_lodTrackBarControl[3],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[3],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val1 = SendMessage(g_lodTrackBarControl[4],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[5],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[4],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[4],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[5],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[5],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[4])
                {
                    int val1 = SendMessage(g_lodTrackBarControl[4],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val1));
                    SendMessage(g_lodEditControl[4],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[3],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[5],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[3],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[3],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[5],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[5],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[5])
                {
                    int val2 = SendMessage(g_lodTrackBarControl[5],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val2));
                    SendMessage(g_lodEditControl[5],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[3],TBM_GETPOS,0,0);
                    int val1 = SendMessage(g_lodTrackBarControl[4],TBM_GETPOS,0,0);
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[3],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[3],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[4],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[4],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[6])
                {
                    int val0 = SendMessage(g_lodTrackBarControl[6],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[6],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val1 = SendMessage(g_lodTrackBarControl[7],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[8],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[7],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[7],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[8],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[8],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[7])
                {
                    int val1 = SendMessage(g_lodTrackBarControl[7],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val1));
                    SendMessage(g_lodEditControl[7],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[6],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[8],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[6],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[6],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[8],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[8],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[8])
                {
                    int val2 = SendMessage(g_lodTrackBarControl[8],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val2));
                    SendMessage(g_lodEditControl[8],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[6],TBM_GETPOS,0,0);
                    int val1 = SendMessage(g_lodTrackBarControl[7],TBM_GETPOS,0,0);
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[6],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[6],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[7],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[7],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[9])
                {
                    int val0 = SendMessage(g_lodTrackBarControl[9],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[9],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val1 = SendMessage(g_lodTrackBarControl[10],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[11],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[10],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[10],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[11],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[11],TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[10])
                {
                    int val1 = SendMessage(g_lodTrackBarControl[10],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val1));
                    SendMessage(g_lodEditControl[10],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[9],TBM_GETPOS,0,0);
                    int val2 = SendMessage(g_lodTrackBarControl[11],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(g_lodEditControl[9],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[9],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[11],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[11],TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[11])
                {
                    int val2 = SendMessage(g_lodTrackBarControl[11],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val2));
                    SendMessage(g_lodEditControl[11],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(g_lodTrackBarControl[9],TBM_GETPOS,0,0);
                    int val1 = SendMessage(g_lodTrackBarControl[10],TBM_GETPOS,0,0);
                    if (val0 > val2)
                    {
                        SendMessage(g_lodEditControl[9],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[9],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(g_lodEditControl[10],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(g_lodTrackBarControl[10],TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[12])
                {
                    int val0 = SendMessage(g_lodTrackBarControl[12],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[12],WM_SETTEXT,0,(LPARAM)buf);
                }
                else if ((HWND)lParam == g_lodTrackBarControl[13])
                {
                    int val0 = SendMessage(g_lodTrackBarControl[13],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[13],WM_SETTEXT,0,(LPARAM)buf);
                }

                // Active Player 1
                else if ((HWND)lParam == g_lodTrackBarControl[14])
                {
                    int val0 = SendMessage(
                            g_lodTrackBarControl[14],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[14],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val1 = SendMessage(
                            g_lodTrackBarControl[15],TBM_GETPOS,0,0);
                    int val2 = SendMessage(
                            g_lodTrackBarControl[16],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(
                                g_lodEditControl[15],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[15],
                                TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                    if (val0 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[16],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[16],
                                TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[15])
                {
                    int val1 = SendMessage(
                            g_lodTrackBarControl[15],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val1));
                    SendMessage(
                            g_lodEditControl[15],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(
                            g_lodTrackBarControl[14],TBM_GETPOS,0,0);
                    int val2 = SendMessage(
                            g_lodTrackBarControl[16],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(
                                g_lodEditControl[14],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[14],
                                TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[16],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[16],
                                TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[16])
                {
                    int val2 = SendMessage(
                            g_lodTrackBarControl[16],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val2));
                    SendMessage(
                            g_lodEditControl[16],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(
                            g_lodTrackBarControl[14],TBM_GETPOS,0,0);
                    int val1 = SendMessage(
                            g_lodTrackBarControl[15],TBM_GETPOS,0,0);
                    if (val0 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[14],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[14],
                                TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[15],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[15],
                                TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                }

                // Active Player 2
                else if ((HWND)lParam == g_lodTrackBarControl[17])
                {
                    int val0 = SendMessage(
                            g_lodTrackBarControl[17],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val0));
                    SendMessage(g_lodEditControl[17],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val1 = SendMessage(
                            g_lodTrackBarControl[18],TBM_GETPOS,0,0);
                    int val2 = SendMessage(
                            g_lodTrackBarControl[19],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(
                                g_lodEditControl[18],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[18],
                                TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                    if (val0 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[19],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[19],
                                TBM_SETPOS,TRUE,(LPARAM)val0);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[18])
                {
                    int val1 = SendMessage(
                            g_lodTrackBarControl[18],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val1));
                    SendMessage(
                            g_lodEditControl[18],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(
                            g_lodTrackBarControl[17],TBM_GETPOS,0,0);
                    int val2 = SendMessage(
                            g_lodTrackBarControl[19],TBM_GETPOS,0,0);
                    if (val0 > val1)
                    {
                        SendMessage(
                                g_lodEditControl[17],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[17],
                                TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[19],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[19],
                                TBM_SETPOS,TRUE,(LPARAM)val1);
                    }
                }
                else if ((HWND)lParam == g_lodTrackBarControl[19])
                {
                    int val2 = SendMessage(
                            g_lodTrackBarControl[19],TBM_GETPOS,0,0);
                    char buf[20];
                    sprintf(buf,"%0.3f",getSwitchValue(val2));
                    SendMessage(
                            g_lodEditControl[19],WM_SETTEXT,0,(LPARAM)buf);

                    // adjust other sliders, if necessary
                    int val0 = SendMessage(
                            g_lodTrackBarControl[17],TBM_GETPOS,0,0);
                    int val1 = SendMessage(
                            g_lodTrackBarControl[18],TBM_GETPOS,0,0);
                    if (val0 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[17],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[17],
                                TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                    if (val1 > val2)
                    {
                        SendMessage(
                                g_lodEditControl[18],WM_SETTEXT,0,(LPARAM)buf);
                        SendMessage(
                                g_lodTrackBarControl[18],
                                TBM_SETPOS,TRUE,(LPARAM)val2);
                    }
                }

            }
            break;

		case WM_APP_EXECHANGE:
			if (wParam == VK_RETURN) {
				g_isBeingEdited = FALSE;
				MessageBox(hWnd, "WM_APP_EXECHANGE", "Installer Message", 0);
			}
			break;
	}
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

bool InitApp(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	WNDCLASSEX wcx;

	// cbSize - the size of the structure.
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = (WNDPROC)WindowProc1;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = "LODCFGCLS";
	wcx.hIconSm = NULL;

	// Register the class with Windows
	if(!RegisterClassEx(&wcx))
		return false;

	return true;
}

HWND BuildWindow(int nCmdShow)
{
	DWORD style, xstyle;
	HWND retval;

	style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	xstyle = WS_EX_LEFT;

    RECT r = {0};
    GetWindowRect(GetDesktopWindow(), &r);
    int x = (r.left+r.right)/2 - WIN_WIDTH/2;
    int y = (r.top+r.bottom)/2 - WIN_HEIGHT/2;

	retval = CreateWindowEx(xstyle,
        "LODCFGCLS",      // class name
        LODCFG_WINDOW_TITLE, // title for our window (appears in the titlebar)
        style,
        x, //CW_USEDEFAULT,  // initial x coordinate
        y, //CW_USEDEFAULT,  // initial y coordinate
        WIN_WIDTH, WIN_HEIGHT,   // width and height of the window
        NULL,           // no parent window.
        NULL,           // no menu
        NULL,           // no creator
        NULL);          // no extra data

	if (retval == NULL) return NULL;  // BAD.

	ShowWindow(retval,nCmdShow);  // Show the window
	return retval; // return its handle for future use.
}

void SaveConfig()
{
    UpdateConfig(_lmconfig);
    if (writeConfig(CFG_FILE))
    {
		// show message box with success msg
		char buf[BUFLEN];
		ZeroMemory(buf, BUFLEN);
		sprintf(buf, "\
======= SUCCESS! ========\n\
Kitserver configuration saved.\n\
\n");
		MessageBox(hWnd, buf, "Configuration Tool Message", 0);

    } else {
		// show message box with error msg
		char buf[BUFLEN];
		ZeroMemory(buf, BUFLEN);
		sprintf(buf, "\
========== ERROR! ===========\n\
Problem saving Kitserver configuration info this file:\n\
%s.\n\
\n", CFG_FILE);

		MessageBox(hWnd, buf, "Configuration Tool Message", 0);
		return;
	}
}

void UpdateControls(LMCONFIG& cfg)
{
    // Aspect Ratio
    SendMessage(g_arCheckBox, BM_SETCHECK, BST_CHECKED, 0);
    EnableWindow(g_arRadio1, true);
    EnableWindow(g_arRadio2, true);
    if (!cfg.aspectRatioCorrectionEnabled) 
    {
        SendMessage(g_arCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
        EnableWindow(g_arRadio1, false);
        EnableWindow(g_arRadio2, false);
        EnableWindow(g_arEditControl, false);
    }
    SendMessage(g_arRadio1, BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(g_arRadio2, BM_SETCHECK, BST_UNCHECKED, 0);
    EnableWindow(g_arEditControl, false);
    if (cfg.screen.aspectRatio > 0.0f)
    {
        SendMessage(g_arRadio1, BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(g_arRadio2, BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(g_arEditControl, true);
        char buf[40] = {0}; sprintf(buf,"%0.5f",cfg.screen.aspectRatio);
        SendMessage(g_arEditControl, WM_SETTEXT, 0, (LPARAM)buf);
    } 

    // Resolution
    SendMessage(g_resCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    EnableWindow(g_resWidthControl, false);
    EnableWindow(g_resHeightControl, false);
    if (cfg.screen.width > 0 && cfg.screen.height > 0)
    {
        SendMessage(g_resCheckBox, BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(g_resWidthControl, true);
        char buf[40] = {0}; sprintf(buf,"%d",cfg.screen.width);
        SendMessage(g_resWidthControl, WM_SETTEXT, 0, (LPARAM)buf);
        EnableWindow(g_resHeightControl, true);
        sprintf(buf,"%d",cfg.screen.height);
        SendMessage(g_resHeightControl, WM_SETTEXT, 0, (LPARAM)buf);
    }

    // Picture quality
    SendMessage(g_pqCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    for (int i=0; i<3; i++) {
        EnableWindow(g_pqRadio[i], false);
    }
    if (cfg.pictureQuality >= 0) {
        SendMessage(g_pqCheckBox, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=0; i<3; i++) {
            EnableWindow(g_pqRadio[i], true);
            SendMessage(g_pqRadio[i], BM_SETCHECK, BST_UNCHECKED, 0);
        }
        int x = (cfg.pictureQuality<3)?cfg.pictureQuality:0;
        SendMessage(g_pqRadio[x], BM_SETCHECK, BST_CHECKED, 0);
    }

    // Camera angle
    {
        char buf[40] = {0}; sprintf(buf,"%d",_cameraAngle);
        SendMessage(g_angleControl, WM_SETTEXT, 0, (LPARAM)buf);
    }

    // Match time
    SendMessage(g_timeCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    EnableWindow(g_timeControl, false);
    if (_matchTime != 0)
    {
        SendMessage(g_timeCheckBox, BM_SETCHECK, BST_CHECKED, 0);
        char buf[40] = {0}; sprintf(buf,"%d",_matchTime);
        SendMessage(g_timeControl, WM_SETTEXT, 0, (LPARAM)buf);
        EnableWindow(g_timeControl, true);
    }

    // Game speed
    SendMessage(g_speedCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    EnableWindow(g_speedTrackBarControl, false);
    EnableWindow(g_speedEditControl, false);
    if (_speedFactor > 0.0f)  
    {
        SendMessage(g_speedCheckBox, BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(g_speedTrackBarControl, true);
        EnableWindow(g_speedEditControl, true);

        float sw = min(getMaxSwitchValue2(),max(getMinSwitchValue2(),
                    _speedFactor));
        char buf[40] = {0}; sprintf(buf,"%0.2f",sw);
        SendMessage(g_speedTrackBarControl, TBM_SETPOS, TRUE, (LPARAM)getTickValue2(sw));
        SendMessage(g_speedEditControl, WM_SETTEXT, 0, (LPARAM)buf);
        
        // redraw
        ShowWindow(g_speedTrackBarControl, SW_HIDE);
        ShowWindow(g_speedTrackBarControl, SW_SHOW);
    }

    // LOD
    SendMessage(g_lodCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    HWND labels[] = {
        g_lodLabel1, g_lodLabel2, g_lodLabel3, g_lodLabel4,
        g_lodLabel5, g_lodLabel6, g_lodLabel7, g_lodLabel8,
        g_lodLabel9, g_lodLabel10, g_lodLabel11, g_lodLabel12,
        g_lodLabel13, g_lodLabel14, g_lodLabel15, g_lodLabel16,
        g_lodLabel17, g_lodLabel18, g_lodLabel19, g_lodLabel20,
    };
    for (int x=0; x<20; x++)
    {
        EnableWindow(g_lodTrackBarControl[x], false);
        EnableWindow(g_lodEditControl[x], false);
        EnableWindow(labels[x], false);
    }

    if (cfg.lod.lodPlayersInplayS1 > 0.0f)  
    {
        SendMessage(g_lodCheckBox1, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=0; i<3; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }
        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersInplayS1));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[0], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[0], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersInplayS2 > 0.0f)  
    {
        SendMessage(g_lodCheckBox1, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=0; i<3; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }
        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersInplayS2));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[1], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[1], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersInplayS3 > 0.0f)  
    {
        SendMessage(g_lodCheckBox1, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=0; i<3; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }
        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersInplayS3));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[2], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[2], WM_SETTEXT, 0, (LPARAM)buf);
    }

    if (cfg.lod.lodPlayersReplayS1 > 0.0f)  
    {
        SendMessage(g_lodCheckBox2, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=3; i<6; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersReplayS1));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[3], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[3], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersReplayS2 > 0.0f)  
    {
        SendMessage(g_lodCheckBox2, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=3; i<6; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersReplayS2));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[4], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[4], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersReplayS3 > 0.0f)  
    {
        SendMessage(g_lodCheckBox2, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=3; i<6; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersReplayS3));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[5], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[5], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersEntranceS1 > 0.0f)  
    {
        SendMessage(g_lodCheckBox3, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=6; i<9; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersEntranceS1));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[6], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[6], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersEntranceS2 > 0.0f)  
    {
        SendMessage(g_lodCheckBox3, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=6; i<9; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersEntranceS2));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[7], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[7], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersEntranceS3 > 0.0f)  
    {
        SendMessage(g_lodCheckBox3, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=6; i<9; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersEntranceS3));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[8], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[8], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersMiscS1 > 0.0f)  
    {
        SendMessage(g_lodCheckBox4, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=9; i<12; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersMiscS1));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[9], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[9], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersMiscS2 > 0.0f)  
    {
        SendMessage(g_lodCheckBox4, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=9; i<12; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersMiscS2));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[10], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[10], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodPlayersMiscS3 > 0.0f)  
    {
        SendMessage(g_lodCheckBox4, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=9; i<12; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodPlayersMiscS3));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[11], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[11], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodRefInplay > 0.0f)  
    {
        SendMessage(g_lodCheckBox5, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=12; i<13; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodRefInplay));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[12], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[12], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodRefReplay > 0.0f)  
    {
        SendMessage(g_lodCheckBox6, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=13; i<14; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodRefReplay));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[13], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[13], WM_SETTEXT, 0, (LPARAM)buf);
    }

    if (cfg.lod.lodActivePlayerCKs1 > 0.0f)  
    {
        SendMessage(g_lodCheckBox7, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=14; i<17; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodActivePlayerCKs1));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[14], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[14], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodActivePlayerCKs2 > 0.0f)  
    {
        SendMessage(g_lodCheckBox7, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=14; i<17; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodActivePlayerCKs2));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[15], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[15], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodActivePlayerCKs3 > 0.0f)  
    {
        SendMessage(g_lodCheckBox7, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=14; i<17; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodActivePlayerCKs3));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[16], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[16], WM_SETTEXT, 0, (LPARAM)buf);
    }

    if (cfg.lod.lodActivePlayerFKs1 > 0.0f)  
    {
        SendMessage(g_lodCheckBox8, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=17; i<20; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodActivePlayerFKs1));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[17], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[17], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodActivePlayerFKs2 > 0.0f)  
    {
        SendMessage(g_lodCheckBox8, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=17; i<20; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodActivePlayerFKs2));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[18], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[18], WM_SETTEXT, 0, (LPARAM)buf);
    }
    if (cfg.lod.lodActivePlayerFKs3 > 0.0f)  
    {
        SendMessage(g_lodCheckBox8, BM_SETCHECK, BST_CHECKED, 0);
        for (int i=17; i<20; i++)
        {
            EnableWindow(g_lodTrackBarControl[i], true);
            EnableWindow(g_lodEditControl[i], true);
            EnableWindow(labels[i], true);
        }

        float sw = min(getMaxSwitchValue(),max(getMinSwitchValue(),cfg.lod.lodActivePlayerFKs3));
        char buf[40] = {0}; sprintf(buf,"%0.3f",sw);
        SendMessage(g_lodTrackBarControl[19], TBM_SETPOS, TRUE, (LPARAM)getTickValue(sw));
        SendMessage(g_lodEditControl[19], WM_SETTEXT, 0, (LPARAM)buf);
    }

    // Controller check
    SendMessage(g_controllerCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    if (cfg.controllerCheckEnabled)
        SendMessage(g_controllerCheckBox, BM_SETCHECK, BST_CHECKED, 0);

    /*
    // LOD check
    SendMessage(g_lodCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    if (!cfg.lodCheck1)
        SendMessage(g_lodCheckBox, BM_SETCHECK, BST_CHECKED, 0);
    */
}

void UpdateConfig(LMCONFIG& cfg)
{
    float val;
    int ival;
    char abuf[40] = {0};
    wchar_t buf[40] = {0};

    // Aspect ratio
    _removeConfig("lodmixer", "screen.aspect-ratio");
    bool arChecked = SendMessage(g_arCheckBox, BM_GETCHECK, 0, 0);
    _setConfig("lodmixer", "aspect-ratio.correction.enabled", 
            (arChecked)?wstring(L"1"):wstring(L"0"));
    if (arChecked)
    {
        bool manual = SendMessage(g_arRadio2, BM_GETCHECK, 0, 0);
        if (manual)
        {
            ZeroMemory(abuf, sizeof(abuf));
            SendMessage(g_arEditControl, WM_GETTEXT, sizeof(abuf), (LPARAM)abuf);
            if (sscanf(abuf,"%f",&val)==1 && val>0)
            {
                swprintf(buf,L"%0.5f",val);
                _setConfig("lodmixer", "screen.aspect-ratio", wstring(buf));
            }
        }
    }

    // Custom resolution
    _removeConfig("lodmixer", "screen.width");
    _removeConfig("lodmixer", "screen.height");
    bool resChecked = SendMessage(g_resCheckBox, BM_GETCHECK, 0, 0);
    if (resChecked)
    {
        ZeroMemory(abuf, sizeof(abuf));
        SendMessage(g_resWidthControl, WM_GETTEXT, sizeof(abuf), (LPARAM)abuf);
        if (sscanf(abuf,"%d",&ival)==1 && ival>0)
        {
            swprintf(buf,L"%d",ival);
            _setConfig("lodmixer", "screen.width", wstring(buf));
        }
        ZeroMemory(abuf, sizeof(abuf));
        SendMessage(g_resHeightControl, WM_GETTEXT, sizeof(abuf), (LPARAM)abuf);
        if (sscanf(abuf,"%d",&ival)==1 && ival>0)
        {
            swprintf(buf,L"%d",ival);
            _setConfig("lodmixer", "screen.height", wstring(buf));
        }
    }

    // Picture quality
    _removeConfig("lodmixer", "picture.quality");
    bool pqChecked = SendMessage(g_pqCheckBox, BM_GETCHECK, 0, 0);
    if (pqChecked) {
        int value = -1;
        for (int i=0; i<3; i++) {
            bool checked = SendMessage(g_pqRadio[i], BM_GETCHECK, 0, 0);
            if (checked) {
                value = i;
                break;
            }
        }
        if (value == 0) {
            _setConfig("lodmixer", "picture.quality", wstring(L"0"));
        }
        else if (value == 1) {
            _setConfig("lodmixer", "picture.quality", wstring(L"1"));
        }
        else if (value == 2) {
            _setConfig("lodmixer", "picture.quality", wstring(L"2"));
        }
    }

    // Camera angle
    _removeConfig("camera", "angle");
    {
        ZeroMemory(abuf, sizeof(abuf));
        SendMessage(g_angleControl, WM_GETTEXT, sizeof(abuf), (LPARAM)abuf);
        if (sscanf(abuf,"%d",&ival)==1 && ival>0)
        {
            swprintf(buf,L"%d",ival);
            _setConfig("camera", "angle", wstring(buf));
        }
    }

    // Match time
    _removeConfig("time", "match.time");
    bool mtChecked = SendMessage(g_timeCheckBox, BM_GETCHECK, 0, 0);
    if (mtChecked)
    {
        ZeroMemory(abuf, sizeof(abuf));
        SendMessage(g_timeControl, WM_GETTEXT, sizeof(abuf), (LPARAM)abuf);
        if (sscanf(abuf,"%d",&ival)==1 && ival>0 && ival<=255)
        {
            swprintf(buf,L"%d",ival);
            _setConfig("time", "match.time", wstring(buf));
        }
        else
        {
            _setConfig("time", "match.time", wstring(L"10"));
            sprintf(abuf,"%d",10);
            SendMessage(g_timeControl, WM_SETTEXT, 0, (LPARAM)abuf);
        }
    }

    // Game speed
    _removeConfig("speeder", "count.factor");
    bool gsChecked = SendMessage(g_speedCheckBox, BM_GETCHECK, 0, 0);
    if (gsChecked)
    {
        val = getSwitchValue2(SendMessage(g_speedTrackBarControl, 
                    TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.2f",val);
        _setConfig("speeder", "count.factor", wstring(buf));
    }

    // LOD
    bool lodChecked;
    lodChecked = SendMessage(g_lodCheckBox1, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[0], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.inplay.s1", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[1], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.inplay.s2", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[2], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.inplay.s3", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.players.inplay.s1");
        _removeConfig("lodmixer", "lod.players.inplay.s2");
        _removeConfig("lodmixer", "lod.players.inplay.s3");
    }

    lodChecked = SendMessage(g_lodCheckBox2, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[3], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.replay.s1", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[4], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.replay.s2", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[5], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.replay.s3", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.players.replay.s1");
        _removeConfig("lodmixer", "lod.players.replay.s2");
        _removeConfig("lodmixer", "lod.players.replay.s3");
    }

    lodChecked = SendMessage(g_lodCheckBox3, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[6], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.entrance.s1", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[7], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.entrance.s2", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[8], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.entrance.s3", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.players.entrance.s1");
        _removeConfig("lodmixer", "lod.players.entrance.s2");
        _removeConfig("lodmixer", "lod.players.entrance.s3");
    }

    lodChecked = SendMessage(g_lodCheckBox4, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[9], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.misc.s1", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[10], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.misc.s2", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[11], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.players.misc.s3", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.players.misc.s1");
        _removeConfig("lodmixer", "lod.players.misc.s2");
        _removeConfig("lodmixer", "lod.players.misc.s3");
    }

    lodChecked = SendMessage(g_lodCheckBox7, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[14], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.active.player.ck.s1", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[15], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.active.player.ck.s2", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[16], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.active.player.ck.s3", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.active.player.ck.s1");
        _removeConfig("lodmixer", "lod.active.player.ck.s2");
        _removeConfig("lodmixer", "lod.active.player.ck.s3");
    }

    lodChecked = SendMessage(g_lodCheckBox8, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[17], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.active.player.fk.s1", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[18], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.active.player.fk.s2", wstring(buf));

        val = getSwitchValue(SendMessage(g_lodTrackBarControl[19], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.active.player.fk.s3", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.active.player.fk.s1");
        _removeConfig("lodmixer", "lod.active.player.fk.s2");
        _removeConfig("lodmixer", "lod.active.player.fk.s3");
    }

    lodChecked = SendMessage(g_lodCheckBox5, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[12], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.ref.inplay", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.ref.inplay");
    }

    lodChecked = SendMessage(g_lodCheckBox6, BM_GETCHECK, 0, 0);
    if (lodChecked)
    {
        val = getSwitchValue(SendMessage(g_lodTrackBarControl[13], TBM_GETPOS, 0, 0));
        swprintf(buf,L"%0.3f",val);
        _setConfig("lodmixer", "lod.ref.replay", wstring(buf));
    }
    else
    {
        _removeConfig("lodmixer", "lod.ref.replay");
    }

    // Controller check
    bool controllerChecked = SendMessage(g_controllerCheckBox, BM_GETCHECK, 0, 0);
    if (controllerChecked)
    {
        _setConfig("sides", "free.select", wstring(L"1"));
    }
    else
    {
        _removeConfig("sides", "free.select");
    }
}

/*
    initialize common controls
*/
void InitComCtls()
{
    INITCOMMONCONTROLSEX icce = {0};

    icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icce.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icce);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg; int retval;

    // init common controls
    InitComCtls();

 	if(InitApp(hInstance, lpCmdLine) == false)
		return 0;

	hWnd = BuildWindow(nCmdShow);
	if(hWnd == NULL)
		return 0;

	// build GUI
	if (!BuildControls(hWnd))
		return 0;

    if (!readConfig(CFG_FILE))       // try config.txt - if not there
        readConfig(CFG_DEFAULT_FILE); // then try default config
    _getConfig("lodmixer", "screen.width", DT_DWORD, 1, lodmixerConfig);
    _getConfig("lodmixer", "screen.height", DT_DWORD, 2, lodmixerConfig);
    _getConfig("lodmixer", "screen.aspect-ratio", DT_FLOAT, 3, lodmixerConfig);
    _getConfig("lodmixer", "aspect-ratio.correction.enabled", DT_DWORD, 6, lodmixerConfig);
    _getConfig("sides", "free.select", DT_DWORD, 7, lodmixerConfig);
    _getConfig("lodmixer", "lod.check1", DT_DWORD, 8, lodmixerConfig);
    //_getConfig("camera", "angle", DT_DWORD, 9, lodmixerConfig);
    _getConfig("time", "match.time", DT_DWORD, 10, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.entrance.s1", DT_FLOAT, 11, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.entrance.s2", DT_FLOAT, 12, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.entrance.s3", DT_FLOAT, 23, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.inplay.s1", DT_FLOAT, 13, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.inplay.s2", DT_FLOAT, 14, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.inplay.s3", DT_FLOAT, 15, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.misc.s1", DT_FLOAT, 16, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.misc.s2", DT_FLOAT, 17, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.misc.s3", DT_FLOAT, 24, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.replay.s1", DT_FLOAT, 18, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.replay.s2", DT_FLOAT, 19, lodmixerConfig);
    _getConfig("lodmixer", "lod.players.replay.s3", DT_FLOAT, 20, lodmixerConfig);
    _getConfig("lodmixer", "lod.ref.inplay", DT_FLOAT, 21, lodmixerConfig);
    _getConfig("lodmixer", "lod.ref.replay", DT_FLOAT, 22, lodmixerConfig);
    _getConfig("speeder", "count.factor", DT_FLOAT, 25, lodmixerConfig);
    _getConfig("lodmixer", 
            "lod.active.player.ck.s1", DT_FLOAT, 26, lodmixerConfig);
    _getConfig("lodmixer", 
            "lod.active.player.ck.s2", DT_FLOAT, 27, lodmixerConfig);
    _getConfig("lodmixer", 
            "lod.active.player.ck.s3", DT_FLOAT, 28, lodmixerConfig);
    _getConfig("lodmixer", 
            "lod.active.player.fk.s1", DT_FLOAT, 29, lodmixerConfig);
    _getConfig("lodmixer", 
            "lod.active.player.fk.s2", DT_FLOAT, 30, lodmixerConfig);
    _getConfig("lodmixer", 
            "lod.active.player.fk.s3", DT_FLOAT, 31, lodmixerConfig);
    _getConfig("lodmixer", "picture.quality", DT_DWORD, 32, lodmixerConfig);

    UpdateControls(_lmconfig);

	// show credits
	char buf[BUFLEN];
	ZeroMemory(buf, BUFLEN);
	strncpy(buf, CREDITS, BUFLEN-1);
	SendMessage(g_statusTextControl, WM_SETTEXT, 0, (LPARAM)buf);

	while((retval = GetMessage(&msg,NULL,0,0)) != 0)
	{
		if(retval == -1)
			return 0;	// an error occured while getting a message

		if (!IsDialogMessage(hWnd, &msg)) // need to call this to make WS_TABSTOP work
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

void lodmixerConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1:	// width
			_lmconfig.screen.width = *(DWORD*)pValue;
			break;
		case 2: // height
			_lmconfig.screen.height = *(DWORD*)pValue;
			break;
		case 3: // aspect ratio
			_lmconfig.screen.aspectRatio = *(float*)pValue;
			break;
		case 6: // LOD
			_lmconfig.aspectRatioCorrectionEnabled = *(DWORD*)pValue != 0;
			break;
		case 7: // Controller check
			_lmconfig.controllerCheckEnabled = *(DWORD*)pValue != 0;
			break;
		case 8: // Controller check
			_lmconfig.lodCheck1 = *(DWORD*)pValue != 0;
			break;
		case 9: // Camera angle
			_cameraAngle = *(DWORD*)pValue;
            break;
		case 10: // Match time
			_matchTime = *(DWORD*)pValue;
			break;
		case 11: 
			_lmconfig.lod.lodPlayersEntranceS1 = *(float*)pValue;
			break;
		case 12: 
			_lmconfig.lod.lodPlayersEntranceS2 = *(float*)pValue;
			break;
		case 13: 
			_lmconfig.lod.lodPlayersInplayS1 = *(float*)pValue;
			break;
		case 14: 
			_lmconfig.lod.lodPlayersInplayS2 = *(float*)pValue;
			break;
		case 15: 
			_lmconfig.lod.lodPlayersInplayS3 = *(float*)pValue;
			break;
		case 16: 
			_lmconfig.lod.lodPlayersMiscS1 = *(float*)pValue;
			break;
		case 17: 
			_lmconfig.lod.lodPlayersMiscS2 = *(float*)pValue;
			break;
		case 18: 
			_lmconfig.lod.lodPlayersReplayS1 = *(float*)pValue;
			break;
		case 19: 
			_lmconfig.lod.lodPlayersReplayS2 = *(float*)pValue;
			break;
		case 20: 
			_lmconfig.lod.lodPlayersReplayS3 = *(float*)pValue;
			break;
		case 21: 
			_lmconfig.lod.lodRefInplay = *(float*)pValue;
			break;
		case 22: 
			_lmconfig.lod.lodRefReplay = *(float*)pValue;
			break;
		case 23: 
			_lmconfig.lod.lodPlayersEntranceS3 = *(float*)pValue;
			break;
		case 24: 
			_lmconfig.lod.lodPlayersMiscS3 = *(float*)pValue;
			break;
		case 25: 
			_speedFactor = *(float*)pValue;
			break;
		case 26: 
			_lmconfig.lod.lodActivePlayerCKs1 = *(float*)pValue;
			break;
		case 27: 
			_lmconfig.lod.lodActivePlayerCKs2 = *(float*)pValue;
			break;
		case 28: 
			_lmconfig.lod.lodActivePlayerCKs3 = *(float*)pValue;
			break;
		case 29: 
			_lmconfig.lod.lodActivePlayerFKs1 = *(float*)pValue;
			break;
		case 30: 
			_lmconfig.lod.lodActivePlayerFKs2 = *(float*)pValue;
			break;
		case 31: 
			_lmconfig.lod.lodActivePlayerFKs3 = *(float*)pValue;
			break;
		case 32: 
			_lmconfig.pictureQuality = *(DWORD*)pValue;
			break;
	}
}

