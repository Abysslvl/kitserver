// ADDRESSES for afs2fs.cpp
BYTE allowedGames[] = 
{
    gvPES2013demo,
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 1
enum 
{ 
    C_READ_BALLS 
};

#define NOCODEADDR {0},
DWORD codeArray[][CODELEN] = 
{ 
    // PES2013 demo
    {
        0x431553
    },
    // PES2013 v1.00
    NOCODEADDR
    // PES2013 v1.01
    NOCODEADDR
    // PES2013 v1.02
    NOCODEADDR
    // PES2013 v1.03
    {
        0x4302A3
    },
    // PES2013 v1.04
    {
        0x430423
    }
};

#define DATALEN 5 
enum 
{
    BIN_SIZES_TABLE, SONGS_INFO_TABLE, NUM_SONGS, NUM_BALLS, PES_DIR,
};

#define NODATAADDR {0,0,0,0,0},
DWORD dataArray[][DATALEN] = 
{
    // PES2013 demo
    {
        0x19972E0, 0x13A91D0, 28, 30, 0,
    },
    // PES2013 v1.00
    NODATAADDR
    // PES2013 v1.01
    NODATAADDR
    // PES2013 v1.02
    NODATAADDR
    // PES2013 v1.03
    {
        0x1A40BC0, 0x144B8F0, 28, 30, 0,
    },
    // PES2013 v1.04
    {
        0x1A4CF00, 0x144EA70, 28, 30, 0,
    }
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
