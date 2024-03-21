// ADDRESSES for speeder module
BYTE allowedGames[] = {
    gvPES2013demo,
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 1
enum { 
    CS_QUERY_PERFORMANCE_FREQUENCY,
};

#define NOCODEADDR {0},
DWORD codeArray[][CODELEN] = { 
    // PES2013 demo
    {
        0x10A3099,
    },
    // PES2013 v1.00
    NOCODEADDR
    // PES2013 v1.01
    NOCODEADDR
    // PES2013 v1.02
    NOCODEADDR
    // PES2013 v1.03
    {
        0x1138199,
    },
    // PES2013 v1.04
    {
        0x113B7F9,
    }
};

#define DATALEN 1 
enum {
    DUMMY
};

#define NODATAADDR {0},
DWORD dataArray[][DATALEN] = {
    // PES2013 demo
    NODATAADDR
    // PES2013 v1.00
    NODATAADDR
    // PES2013 v1.01
    NODATAADDR
    // PES2013 v1.02
    NODATAADDR
    // PES2013 v1.03
    NODATAADDR
    // PES2013 v1.04
    NODATAADDR
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
