// win32gui.h

#ifndef _JUCE_WIN32GUI
#define _JUCE_WIN32GUI

#include <windows.h>

#define WIN_WIDTH 540 
//#define WIN_WIDTH 600 
//#define WIN_HEIGHT 315
//#define WIN_HEIGHT 310
//#define WIN_HEIGHT 690
//#define WIN_HEIGHT 660
//#define WIN_HEIGHT 777
#define WIN_HEIGHT 580

extern HWND hTab;        // our tab control
extern HWND hTabView1;   // view window for tab1
extern HWND hTabView2;   // view window for tab2

extern HWND g_lodListControl[5];          // lod lists
extern HWND g_crowdCheckBox;              // crowd
extern HWND g_JapanCheckBox;              // Japan check

extern HWND g_weatherListControl;         // weather (default,fine,rainy,random)
extern HWND g_timeListControl;            // time of the day (default,day,night)
extern HWND g_seasonListControl;          // season (default,summer,winter)
extern HWND g_stadiumEffectsListControl;  // stadium effects (default,0/1)
extern HWND g_numberOfSubs;               // number of substitutions
extern HWND g_homeCrowdListControl;       // home crowd attendance (default,0-3)
extern HWND g_awayCrowdListControl;       // away crowd attendance (default,0-3)
extern HWND g_crowdStanceListControl;      // crowd stance (default,1-3)

extern HWND g_resCheckBox;
extern HWND g_resWidthControl;
extern HWND g_resHeightControl;
extern HWND g_arCheckBox;
extern HWND g_arRadio1;
extern HWND g_arRadio2;
extern HWND g_pqCheckBox;
extern HWND g_pqRadio[3];
extern HWND g_arEditControl;
extern HWND g_angleControl;
extern HWND g_timeCheckBox;
extern HWND g_timeControl;
extern HWND g_lodCheckBox;
extern HWND g_controllerCheckBox;
extern HWND g_defLodControl;
extern HWND g_lodCheckBox1;
extern HWND g_lodCheckBox2;
extern HWND g_lodCheckBox3;
extern HWND g_lodCheckBox4;
extern HWND g_lodCheckBox5;
extern HWND g_lodCheckBox6;
extern HWND g_lodCheckBox7;
extern HWND g_lodCheckBox8;
extern HWND g_lodLabel1;
extern HWND g_lodLabel2;
extern HWND g_lodLabel3;
extern HWND g_lodLabel4;
extern HWND g_lodLabel5;
extern HWND g_lodLabel6;
extern HWND g_lodLabel7;
extern HWND g_lodLabel8;
extern HWND g_lodLabel9;
extern HWND g_lodLabel10;
extern HWND g_lodLabel11;
extern HWND g_lodLabel12;
extern HWND g_lodLabel13;
extern HWND g_lodLabel14;
extern HWND g_lodLabel15;
extern HWND g_lodLabel16;
extern HWND g_lodLabel17;
extern HWND g_lodLabel18;
extern HWND g_lodLabel19;
extern HWND g_lodLabel20;
extern HWND g_lodTrackBarControl[20];
extern HWND g_lodEditControl[20];
extern HWND g_speedCheckBox;
extern HWND g_speedTrackBarControl;
extern HWND g_speedEditControl;

extern HWND g_statusTextControl;          // displays status messages
extern HWND g_saveButtonControl;          // save settings button
extern HWND g_defButtonControl;          // default settings button

// functions
bool BuildControls(HWND parent);
int getTickValue(float switchValue);
float getSwitchValue(int tickValue);
float getMaxSwitchValue();
float getMinSwitchValue();
int getTickValue2(float switchValue);
float getSwitchValue2(int tickValue);
float getMaxSwitchValue2();
float getMinSwitchValue2();

void ShowMiscTab(int flag);
void ShowLODTab(int flag);

#endif
