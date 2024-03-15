// ADDRESSES for speeder module
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
    CS_QUERY_PERFORMANCE_FREQUENCY,
};

#define NOCODEADDR {0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    NOCODEADDR,
    // PES2011 v1.01
    NOCODEADDR,
    // PES2011 v1.02
    NOCODEADDR,
    // PES2011 v1.03
    NOCODEADDR,
    // PES2012 demo
    {
        0x10e5279,
    },
    // PES2012 demo2
    {
        0x10f93d9,
    },
    // PES2012
    {
        0x1150799,
    },
    // PES2012 v1.01
    {
        0x1153bb9,
    },
    // PES2012 v1.02
    {
        0x11593c9,
    },
    // PES2012 v1.03
    {
        0x115a049,
    },
    // PES2012 v1.06
    {
        0x115c659,
    },
};

#define DATALEN 1 
enum {
    DUMMY
};

#define NODATAADDR {0}
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    NODATAADDR,
    // PES2011 v1.01
    NODATAADDR,
    // PES2011 v1.02
    NODATAADDR,
    // PES2011 v1.03
    NODATAADDR,
    // PES2012 demo
    NODATAADDR,
    // PES2012 demo2
    NODATAADDR,
    // PES2012
    NODATAADDR,
    // PES2012 v101
    NODATAADDR,
    // PES2012 v102
    NODATAADDR,
    // PES2012 v103
    NODATAADDR,
    // PES2012 v106
    NODATAADDR,
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
