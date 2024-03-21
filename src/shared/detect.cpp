// detect.cpp

#include "windows.h"
#include <stdio.h>
#include "detect.h"
#include "imageutil.h"
#include "shared.h"
#include "manage.h"

wchar_t* GAME[] = {
    L"PES2013 demo",
    L"PES2013 v1.00",
    L"PES2013 v1.01",
    L"PES2013 v1.02",
    L"PES2013 v1.03",
    L"PES2013 v1.04"
};

char* GAME_GUID[] = {
   // "PES2013 demo"
	"1.00",
    // PES2013 v1.00
    "1.00",
    // PES2013 v1.01
    "1.01",
    // PES2013 v1.02
    "1.02",
    // PES2013 v1.03
    "1.03",
    // PES2013 v1.04
    "1.04"
};

DWORD GAME_GUID_OFFSETS[] = { 
    // PES2013 demo
    0x1072EA8,
    // PES2013 v1.00
    0x10F7538,
    // PES2013 v1.01
    0x1117AE0,
    // PES2013 v1.02
    0x1116EE0,
    // PES2013 v1.03
    0x11127A0,
    // PES2013 v1.04
    0x1120108
};

DWORD GAME_GUID_MEMORY[] = { 
    // PES2013 demo
    0x14746A8,
    // PES2013 v1.00
    0x14F9138,
    // PES2013 v1.01
    0x1518EE0,
    // PES2013 v1.02
    0x1517EE0,
    // PES2013 v1.03
    0x1513DA0,
    // PES2013 v1.04
    0x1520F08
};

BYTE BASE_GAME[] = {
    gvPES2013demo,
    gvPES2013v100,
    gvPES2013v101,
    gvPES2013v102,
    gvPES2013v103,
    gvPES2013v104
};

bool ISGAME[] = {
    // PES2013 demo
    true,
    // PES2013 v1.00
    false,
    // PES2013 v1.01
    false,
    // PES2013 v1.02
    false,
    // PES2013 v1.03
    true,
    // PES2013 v1.04
    true
};

// Returns the game real version id
int GetRealGameVersion(void)
{
    for (int i=0; i<sizeof(GAME_GUID)/sizeof(char*); i++)
    {
        if (!IsBadReadPtr((char*)GAME_GUID_MEMORY[i], 
                    strlen(GAME_GUID[i]))) 
        {
            char* guid = (char*)(GAME_GUID_MEMORY[i]);
            if (memcmp(guid, GAME_GUID[i], strlen(GAME_GUID[i]))==0)
                    return i;
        }
    }
    return -1;
}

// Returns the game version id
int GetRealGameVersion(wchar_t* filename)
{
    char guid[512];
    memset(guid,0,sizeof(guid));

    FILE* f = _wfopen(filename, L"rb");
    if (f == NULL)
        return -1;

    // check for regular exes
    for (int i=0; i<sizeof(GAME_GUID)/sizeof(char*); i++)
    {
        fseek(f, GAME_GUID_OFFSETS[i], SEEK_SET);
        if (fread(guid, strlen(GAME_GUID[i]), 1, f)==1)
        {
            if (memcmp(guid, GAME_GUID[i], strlen(GAME_GUID[i]))==0)
            {
                fclose(f);
                return i;
            }
        }
    }

    // unrecognized.
    return -1;
}

bool isGame(int gameVersion)
{
    if (gameVersion == -1) return false;
    return ISGAME[gameVersion];
}

bool isRealGame(int realGameVersion)
{
    if (realGameVersion == -1) return false;
    return ISGAME[GetGameVersion(realGameVersion)];
}

int GetGameVersion()
{
    int v = GetRealGameVersion();
    if (v == -1) return -1;
    return BASE_GAME[v];
}

int GetGameVersion(int realGameVersion)
{
    if (realGameVersion == -1) return -1;
    return BASE_GAME[realGameVersion];
}
