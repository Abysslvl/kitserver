// ADDRESSES for kserv.cpp
BYTE allowedGames[] = {
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 9
enum {
	C_AFTER_READ_NAMES, C_AFTER_APPLY_CONTENT,
    C_READ_NUM_SLOTS1, C_READ_NUM_SLOTS2, C_READ_NUM_SLOTS3,
    C_AFTER_CREATE_TEXTURE, C_KIT_VTABLE, //nice-to-have
    C_READ_UNIFORM_PICK, C_CHECK_KIT_RELOAD_FLAG, //nice-to-have
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0},
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
        0x0, 0x0,
        0x11D332E, 0x11D3DE0, 0x12B93B0,
        0, 0,
        0, 0,
    },
    // PES2013 v1.04 
    {
        0x0, 0x0,
        0x11D6A3E, 0x11D74F0, 0x12BCAC0,
        0, 0,
        0, 0,
    }
};

#define DATALEN 6 
enum {
	NEXT_MATCH_DATA_PTR, PLAYERS_DATA,
    TEAM_NAMES_OFFSET, TEAM_KIT_INFO_OFFSET,
    NUM_SLOTS_PTR, EURO_TEAM_KIT_INFO_PTR,
};

#define NODATAADDR {0,0,0,0,0,0},
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
        0, 0x19DBF14,
        0x29BF88, 0x396EB0,
        0x1697AF8, 0x0,
    },
    // PES2013 v1.04
    {
        0, 0x19E826C,
        0x29BF88, 0x396EB0,
        0x16A3E30, 0x19E8288,
    }
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
