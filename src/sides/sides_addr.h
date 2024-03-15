// ADDRESSES for sides module
BYTE allowedGames[] = {
    //gvPES2011,
    //gvPES2011v101,
    //gvPES2011v102,
    //gvPES2011v103,
    gvPES2012,
    gvPES2012v101,
    gvPES2012v102,
    gvPES2012v103,
    gvPES2012v106,
};

#define CODELEN 4
enum { 
    CS1, CS2, CS3, CS4,
};

#define NOCODEADDR {0,0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    {
        0x4fcbce, 0x4fb79b, 0x4fc41a, 0x4fc38e,
    },
    // PES2011 v1.01
    {
        0x4fcbae, 0x4fb77b, 0x4fc3fa, 0x4fc36e,
    },
    // PES2011 v1.02
    {
        0x4fcc5e, 0x4fb82b, 0x4fc4aa, 0x4fc41e,
    },
    // PES2011 v1.03
    {
        0x4fcc5e, 0x4fb82b, 0x4fc4aa, 0x4fc41e,
    },
    // PES2012 demo
    NOCODEADDR,
    // PES2012 demo2
    NOCODEADDR,
    // PES2012 
    {
        0x510b07, 0x510e8b, 0x51065a, 0x5105ce,
    },
    // PES2012 v1.01
    {
        0x511147, 0x5114cb, 0x510c9a, 0x510c0e,
    },
    // PES2012 v1.02
    {
        0x511317, 0x51169b, 0x510e6a, 0x510dde,
    },
    // PES2012 v1.03
    {
        0x511387, 0x51170b, 0x510eda, 0x510e4e,
    },
    // PES2012 v1.06
    {
        0x511e87, 0x51220b, 0x5119da, 0x51194e,
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
    // PES2012 v1.01
    NODATAADDR,
    // PES2012 v1.02
    NODATAADDR,
    // PES2012 v1.03
    NODATAADDR,
    // PES2012 v1.06
    NODATAADDR,
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
