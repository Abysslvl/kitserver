// ADDRESSES for afs2fs.cpp
BYTE allowedGames[] = {
    gvPES2013demo,
};

#define CODELEN 1
enum { 
    C_READ_BALLS 
};

#define NOCODEADDR {0}
DWORD codeArray[][CODELEN] = { 
    // PES2013 demo
    {
        0x431553,
    },
};

#define DATALEN 5 
enum {
    BIN_SIZES_TABLE, SONGS_INFO_TABLE, NUM_SONGS, NUM_BALLS, PES_DIR,
};

#define NODATAADDR {0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2013 demo
    {
        0x19972E0, 0x13A91D0, 28, 30, 0,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
