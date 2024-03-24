// ADDRESSES for fserv.cpp
BYTE allowedGames[] = {
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 10
enum { 
    C_CHECK_FACE_AND_HAIR_ID, //C_CHECK_FACE_ID, C_CHECK_HAIR_ID,
    C_GET_FACE_BIN, C_GOT_FACE_BIN,
    C_GET_HAIR_BIN, C_GOT_HAIR_BIN,
    C_RESET_HAIR, C_SQUAD_LIST, C_SET_DEFAULT_PLAYER,
    C_FACEHAIR_READ, C_ONLINE_ENTER,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0},
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
        0x11DC261,
        0x11DCC33, 0x11DCC8E,
        0x12B6832, 0x12B68A7,
        0x7427AF, 0, 0x8190B2,
        0x11DBC1B, 0xD313A3,
    },
    // PES2013 v1.04
    {
        0x11DF971,
        0x11E0343, 0x11E039E,
        0x12B9F42, 0x12B9FB7,
        0x74473F, 0, 0x81A342,
        0x11DF32B, 0xD34253,
    }
};

#define DATALEN 2 
enum {
    EDIT_DATA_PTR, MENU_MODE_IDX,
    //PFIRST_FACE, PFIRST_HAIR,
};

#define NODATAADDR {0,0},
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
    {
        0x19DBF14, 0,
    },
    // PES2013 v1.04
    {
        0x19E826C, 0 ,
    }
};

DWORD code[CODELEN];
DWORD dta[DATALEN];