// ADDRESSES for afsio.cpp
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

#define CODELEN 10
enum {
    C_AT_GET_BUFFERSIZE, C_BEFORE_READ, C_BEFORE_READ2,
    C_AFTER_CREATE_EVENT, C_AT_CLOSE_HANDLE, C_AFTER_GET_OFFSET_PAGES, 
    C_AT_GET_SIZE1, C_AT_GET_SIZE2,
    C_AT_GET_IMG_SIZE1, C_AT_GET_IMG_SIZE2,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    {
        0xc349a1, 0x4dd7a9, 0,
        0x4d19e3, 0x4d1c05, 0x4cf58b, 
        0x4cf4f9, 0x4cf510,
        0x4d1a59, 0x4d10fa,
    },
    // PES2011
    {
        0xca54e1, 0x4de6a9, 0,
        0x4d28f3, 0x4d2b15, 0x4d049b, 
        0x4d0409, 0x4d0420,
        0x4d2969, 0x4d200a,
    },
    // PES2011 v1.01
    {
        0xca59f1, 0x4de699, 0,
        0x4d28c3, 0x4d2ae5, 0x4d046b, 
        0x4d03d9, 0x4d03f0,
        0x4d2939, 0x4d1fda,
    },
    // PES2011 v1.02
    {
        0xca5a81, 0x4de6c9, 0,
        0x4d28f3, 0x4d2b15, 0x4d049b, 
        0x4d0409, 0x4d0420,
        0x4d2969, 0x4d200a,
    },
    // PES2011 v1.03
    {
        0xca5a91, 0x4de6c9, 0,
        0x4d28f3, 0x4d2b15, 0x4d049b, 
        0x4d0409, 0x4d0420,
        0x4d2969, 0x4d200a,
    },
    // PES2012 demo
    {
        0xd1bc71, 0x4efbc9, 0x4ef9c9,
        0x4e2343, 0x4e256b, 0x4dfd0f, 
        0x4dfc35, 0x4dfc65,
        0x4e23bc, 0x4e1c6a,
    },
    // PES2012 demo2
    {
        0xd25261, 0x4efe99, 0x4efc99,
        0x4e2633, 0x4e285b, 0x4dffff, 
        0x4dff25, 0x4dff55,
        0x4e26ac, 0x4e1f5a,
    },
    // PES2012 
    {
        0xd723a1, 0x4f0369, 0x4f0169,
        0x4e2ae3, 0x4e2d0b, 0x4e04af, 
        0x4e03d5, 0x4e0405,
        0x4e2b5c, 0x4e240a,
    },
    // PES2012 v1.01
    {
        0xd73f11, 0x4f0789, 0x4f0589,
        0x4e2f23, 0x4e314b, 0x4e08ef,
        0x4e0815, 0x4e0845,
        0x4e2f9c, 0x4e284a,
    },
    // PES2012 v1.02
    {
        0xd74df1, 0x4f09e9, 0x4f07e9,
        0x4e3173, 0x4e339b, 0x4e0b3f,
        0x4e0a65, 0x4e0a95,
        0x4e31ec, 0x4e2a9a,
    },
    // PES2012 v1.03
    {
        0xd755f1, 0x4f0a39, 0x4f0839,
        0x4e31b3, 0x4e33db, 0x4e0b7f,
        0x4e0aa5, 0x4e0ad5,
        0x4e322c, 0x4e2ada,
    },
    // PES2012 v1.06
    {
        0xd78041, 0x4f08a9, 0x4f06a9,
        0x4e3043, 0x4e326b, 0x4e0a0f,
        0x4e0935, 0x4e0965,
        0x4e30bc, 0x4e296a,
    },
};

#define DATALEN 1 
enum {
    BIN_SIZES_TABLE
};

#define NODATAADDR {0},
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    {
        0x1910e80,
    },
    // PES2011
    {
        0x199f220,
    },
    // PES2011 v1.01
    {
        0x199f280,
    },
    // PES2011 v1.02
    {
        0x199f280,
    },
    // PES2011 v1.03
    {
        0x199f280,
    },
    // PES2012 demo
    {
        0x19fab00,
    },
    // PES2012 demo2
    {
        0x1a11160,
    },
    // PES2012
    {
        0x1a75860,
    },
    // PES2012 v1.01
    {
        0x1a86a60,
    },
    // PES2012 v1.02
    {
        0x1a88aa0,
    },
    // PES2012 v1.03
    {
        0x1a8aac0,
    },
    // PES2012 v1.06
    {
        0x1a892a0,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
