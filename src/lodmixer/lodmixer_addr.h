// ADDRESSES for lodmixer.cpp
BYTE allowedGames[] = {
    gvPES2013demo,
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 3
enum {
    C_SETTINGS_READ, C_QUALITY_CHECK, C_QUALITY_CHECK_2,
};

#define NOCODEADDR {0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2013 demo
    {
        0x11A2AB3, 0x11A5BDF, 0x11A5961,
    },
    // PES2013 v1.00
    NOCODEADDR
    // PES2013 v1.01
    NOCODEADDR
    // PES2013 v1.02
    NOCODEADDR
    // PES2013 v1.03
    {
        0x1239AE3, 0x123CD0F, 0x123CA91,
    },
    // PES2013 v1.04
    {
        0x123D1F3, 0x124041F, 0x12401A1,
    }
};

#define DATALEN 9
enum {
    SCREEN_WIDTH, SCREEN_HEIGHT, WIDESCREEN_FLAG,
    RATIO_4on3, RATIO_16on9,
    LOD_PLAYERS_TABLE1, LOD_PLAYERS_TABLE2, 
    LOD_REF_TABLE1, LOD_REF_TABLE2,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2013 demo
    {
        0x1931EB0, 0x1931EB4, 0x1931EB8,
        0x1499E48, 0x149A404,
        0x15F2BC8, 0x15F2C10,
        0x15F20B8, 0x15F20E0,
    },
    // PES2013 v1.00
    NODATAADDR
    // PES2013 v1.01
    NODATAADDR
    // PES2013 v1.02
    NODATAADDR
    // PES2013 v1.03
    {
        0x19DB720, 0x19DB724, 0x19DB728,
        0x15397C0, 0x1539DF0,
        0x1697EB0, 0x1697EF8,
        0x16973A0, 0x16973C8,
    },
    // PES2013 v1.04
    {
        0x19E7A78, 0x19E7A7C, 0x19E7A80,
        0x1546C68, 0x1547298,
        0x16A41E8, 0x16A4230,
        0x16A36D8, 0x16A3700,
    }
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
