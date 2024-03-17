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
    "ee6f5b82-32a5-47b4-b4ca-171b19b4e324",
    // PES2013 v1.01
    "ee6f5b82-32a5-47b4-b4ca-171b19b4e324",
    // PES2013 v1.02
    "ee6f5b82-32a5-47b4-b4ca-171b19b4e324",
    // PES2013 v1.03
    "ee6f5b82-32a5-47b4-b4ca-171b19b4e324",
    // PES2013 v1.04
    "ee6f5b82-32a5-47b4-b4ca-171b19b4e324"
};

DWORD GAME_GUID_OFFSETS[] = { 
    // PES2013 demo
    0x1072EA8,
    // PES2013 v1.00
    0x10ECB60,
    // PES2013 v1.01
    0x110D0F8,
    // PES2013 v1.02
    0x110C4F8,
    // PES2013 v1.03
    0x1107DA8,
    // PES2013 v1.04
    0x1115710
};

DWORD GAME_GUID_MEMORY[] = { 
    // PES2013 demo
    0x14746A8,
    // PES2013 v1.00
    0x14ee760,
    // PES2013 v1.01
    0x150e4f8,
    // PES2013 v1.02
    0x150d4f8,
    // PES2013 v1.03
    0x15093A8,
    // PES2013 v1.04
    0x1516510
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
    true,
    // PES2013 v1.01
    true,
    // PES2013 v1.02
    true,
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
