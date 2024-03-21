// ADDRESSES for sides module
BYTE allowedGames[] = {
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 4
enum { 
    CS1, CS2, CS3, CS4,
};

#define NOCODEADDR {0,0,0,0},
DWORD codeArray[][CODELEN] = { 
    // PES2013 demo 
    NOCODEADDR
    // PES2013 v1.00
    NOCODEADDR
    // PES2013 v1.01
    NOCODEADDR
    // PES2013 v1.02
    NOCODEADDR
    // PES2013 v1.03
    {
        0x531C68, 0x531FFB, 0x53178A, 0x5316FE,
    },
    // PES2013 v1.04
    {
        0x52BC38, 0x52BFCB, 0x52B75A, 0x52B6CE,
    },
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
