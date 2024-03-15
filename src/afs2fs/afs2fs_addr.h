// ADDRESSES for afs2fs.cpp
BYTE allowedGames[] = {
    //gvPES2011demo,
    //gvPES2011,
    //gvPES2011v101,
    //gvPES2011v102,
    //gvPES2011v103,
    gvPES2012demo,
    gvPES2012demo2,
    gvPES2012,
    gvPES2012v101,
    gvPES2012v102,
    gvPES2012v103,
    gvPES2012v106,
};

#define CODELEN 1
enum { 
    C_READ_BALLS 
};

#define NOCODEADDR {0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    {
        0x42d36a,
    },
    // PES2011
    {
        0x42d40a,
    },
    // PES2011 v1.01
    {
        0x42d28a,
    },
    // PES2011 v1.02
    {
        0x42d30a,
    },
    // PES2011 v1.03
    {
        0x42d30a,
    },
    // PES2012 demo
    {
        0x431ae3,
    },
    // PES2012 demo2
    {
        0x431533,
    },
    // PES2012
    {
        0x430f63,
    },
    // PES2012 v1.01
    {
        0x430d63,
    },
    // PES2012 v1.02
    {
        0x431143,
    },
    // PES2012 v1.03
    {
        0x431df3,
    },
    // PES2012 v1.06
    {
        0x430c13,
    },
};

#define DATALEN 5 
enum {
    BIN_SIZES_TABLE, SONGS_INFO_TABLE, NUM_SONGS, NUM_BALLS, PES_DIR,
};

#define NODATAADDR {0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    {
        0x1910e80, 0x12a03c0, 50, 28, 0,
    },
    // PES2011
    {
        0x199f220, 0x13219c0, 50, 28, 0
    },
    // PES2011 v1.01
    {
        0x199f280, 0x1321918, 50, 28, 0
    },
    // PES2011 v1.02
    {
        0x199f280, 0x13218f8, 50, 28, 0
    },
    // PES2011 v1.03
    {
        0x199f280, 0x13218f8, 50, 28, 0
    },
    // PES2012 demo
    {
        0x19fab00, 0x140ae38, 11, 32, 0,
    },
    // PES2012 demo2
    {
        0x1a11160, 0x1420d80, 11, 32, 0,
    },
    // PES2012
    {
        0x1a75860, 0x147d1d8, 28, 32, 0,
    },
    // PES2012 v1.01
    {
        0x1a86a60, 0x1481350, 28, 32, 0,
    },
    // PES2012 v1.02
    {
        0x1a88aa0, 0x1483570, 28, 32, 0,
    },
    // PES2012 v1.03
    {
        0x1a8aac0, 0x14855b8, 28, 32, 0,
    },
    // PES2012 v1.06
    {
        0x1a892a0, 0x1488658, 28, 32, 0,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
