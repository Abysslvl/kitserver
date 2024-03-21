// ADDRESSES for afsio.cpp
BYTE allowedGames[] = 
{
    gvPES2013demo,
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 10
enum 
{
    C_AT_GET_BUFFERSIZE, C_BEFORE_READ, C_BEFORE_READ2,
    C_AFTER_CREATE_EVENT, C_AT_CLOSE_HANDLE, C_AFTER_GET_OFFSET_PAGES, 
    C_AT_GET_SIZE1, C_AT_GET_SIZE2,
    C_AT_GET_IMG_SIZE1, C_AT_GET_IMG_SIZE2,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2013 demo
    {
        0xcba06f, 0x502A09, 0x502809,
        0x4f51a3, 0x4f53cb, 0x4f2b6f,
        0x4f2a95, 0x4f2ac5,
        0x4f521c, 0x4f4aca,
    },
    // PES2013 v1.00
    NOCODEADDR
    // PES2013 v1.01
    NOCODEADDR
    // PES2013 v1.02
    NOCODEADDR
    // PES2013 v1.03
    {
        0xD384EF, 0x505ED9, 0x505CD9,
        0x4F8683, 0x4F88AB, 0x4F603F,
        0x4F5F65, 0x4F5F95,
        0x4F86FC, 0x4F7FAA,
    },
    // PES2013 v1.04
    {
        0xd3b39f, 0x505e39, 0x505c39,
        0x4f85d3, 0x4f87fb, 0x4f5f8f,
        0x4f5eb5, 0x4f5ee5, 
        0x4f864c, 0x4f7efA,
    }
};

#define DATALEN 1 
enum {
    BIN_SIZES_TABLE
};

#define NODATAADDR {0},
DWORD dataArray[][DATALEN] = {
    // PES2013 demo
    {
        0x19972E0
    },
    // PES2013 v1.00
    NODATAADDR
    // PES2013 v1.01
    NODATAADDR
    // PES2013 v1.02
    NODATAADDR
    // PES2013 v1.03
    {
        0x1A40BC0
    },
    // PES2013 v1.04
    {
        0x1A4CF00
    }
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
