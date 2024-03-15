// detect.h

#ifndef _JUCE_DETECT_H
#define _JUCE_DETECT_H

#include "windows.h"
#include <stdio.h>

// Returns the game version id
int GetRealGameVersion(void);
int GetRealGameVersion(wchar_t* filename);
int GetGameVersion();
int GetGameVersion(int realGameVersion);
bool isGame(int gameVersion);
bool isRealGame(int realGameVersion);

#endif

