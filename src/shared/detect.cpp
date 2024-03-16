// detect.cpp

#include "windows.h"
#include <stdio.h>
#include "detect.h"
#include "imageutil.h"
#include "shared.h"
#include "manage.h"

wchar_t* GAME[] = {
    L"PES2013 demo",
};

char* GAME_GUID[] = {
    "1.00",
};

DWORD GAME_GUID_OFFSETS[] = { 
    0x1072EA8,
};

DWORD GAME_GUID_MEMORY[] = { 
    0x14746A8,
};

BYTE BASE_GAME[] = {
    gvPES2013demo,
};

bool ISGAME[] = {
    true,   // PES2013 demo
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
