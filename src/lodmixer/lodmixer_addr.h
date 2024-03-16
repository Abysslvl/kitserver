// ADDRESSES for lodmixer.cpp
BYTE allowedGames[] = {
    gvPES2013demo,
};

#define CODELEN 3
enum {
    C_SETTINGS_READ, C_QUALITY_CHECK, C_QUALITY_CHECK_2,
};

#define NOCODEADDR {0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2013 demo
    {
        0x11a2ab3, 0x11A5BDF, 0x11A5961,
    },
};

#define DATALEN 9
enum {
    SCREEN_WIDTH, SCREEN_HEIGHT, WIDESCREEN_FLAG,
    RATIO_4on3, RATIO_16on9,
    LOD_PLAYERS_TABLE1, LOD_PLAYERS_TABLE2, 
    LOD_REF_TABLE1, LOD_REF_TABLE2,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0}
DWORD dataArray[][DATALEN] = {
    // PES2013 demo
    {
        0x1931EB0, 0x1931EB4, 0x1931EB8,
        0x1499E48, 0x149A404,
        0x15F2BC8, 0x15F2C10,
        0x15F20B8, 0x15F20E0,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
