// detect.cpp

#include "windows.h"
#include <stdio.h>
#include "detect.h"
#include "imageutil.h"
#include "shared.h"
#include "manage.h"

wchar_t* GAME[] = {
    L"PES2011 demo",
    L"PES2011",
    L"PES2011",
    L"PES2011 v1.01",
    L"PES2011 v1.01",
    L"PES2011 v1.02",
    L"PES2011 v1.03",
    L"PES2012 demo",
    L"PES2012 demo2",
    L"PES2012",
    L"PES2012",
    L"PES2012 v1.01",
    L"PES2012 v1.01",
    L"PES2012 v1.02",
    L"PES2012 v1.02",
    L"PES2012 v1.03",
    L"PES2012 v1.03",
    L"PES2012 v1.06",
    L"PES2012 v1.06",
};

char* GAME_GUID[] = {
    "Pro Evolution Soccer 2011 DEMO",
    "Pro Evolution Soccer 2011",
    "Pro Evolution Soccer 2011",
    "Pro Evolution Soccer 2011",
    "Pro Evolution Soccer 2011",
    "1.02",
    "1.03",
    "Pro Evolution Soccer 2012 DEMO",
    "Pro Evolution Soccer 2012 DEMO2",
    "Pro Evolution Soccer 2012",
    "Pro Evolution Soccer 2012",
    "1.01",
    "1.01",
    "1.02",
    "1.02",
    "1.03",
    "1.03",
    "1.06",
    "1.06",
};

DWORD GAME_GUID_OFFSETS[] = { 
    0xfa2f68, 
    0xf23264, 
    0xf23a64, 
    0xf237bc,
    0xf22bbc,
    0x1027ae0,
    0x1027ae0,
    0x10d0140,
    0x10e5810,
    0x111de88,
    0x11445f8,
    0x1150d68,
    0x1150168,
    0x1152bb8,
    0x1151fb8,
    0x11559c8,
    0x1154dc8,
    0x1153238,
    0x1152638,
};

DWORD GAME_GUID_MEMORY[] = { 
    0x13a4968, 
    0x1323a64, 
    0x1323a64, 
    0x13239bc,
    0x13239bc,
    0x1427ae0,
    0x1427ae0,
    0x14d1140,
    0x14e6a10,
    0x151f088,
    0x1544bf8,
    0x1551968,
    0x1551968,
    0x1553bb8,
    0x1553bb8,
    0x1555bc8,
    0x1555bc8,
    0x1553c38,
    0x1553c38,
};

BYTE BASE_GAME[] = {
    gvPES2011demo,
    gvPES2011,
    gvPES2011,
    gvPES2011v101,
    gvPES2011v101,
    gvPES2011v102,
    gvPES2011v103,
    gvPES2012demo,
    gvPES2012demo2,
    gvPES2012,
    gvPES2012,
    gvPES2012v101,
    gvPES2012v101,
    gvPES2012v102,
    gvPES2012v102,
    gvPES2012v103,
    gvPES2012v103,
    gvPES2012v106,
    gvPES2012v106,
};

bool ISGAME[] = {
    true,   // PES2011 demo
    true,   // PES2011
    true,   // PES2011 nodvd
    true,   // PES2011 v1.01 
    true,   // PES2011 v1.01 nodvd
    true,   // PES2011 v1.02
    true,   // PES2011 v1.03
    true,   // PES2012 demo
    true,   // PES2012 demo2
    true,   // PES2012
    true,   // PES2012
    true,   // PES2012 v1.01
    true,   // PES2012 v1.01 reloaded
    true,   // PES2012 v1.02 
    true,   // PES2012 v1.02 reloaded
    true,   // PES2012 v1.03 
    true,   // PES2012 v1.03 reloaded
    true,   // PES2012 v1.06 
    true,   // PES2012 v1.06 reloaded
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
