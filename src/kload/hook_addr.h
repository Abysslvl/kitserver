// ADDRESSES for hook.cpp
#define CODELEN 19
enum {
	C_D3DCREATE_CS,
	C_LOADTEXTUREFORPLAYER_CS, C_LOADTEXTUREFORPLAYER,
	TBL_BEGINRENDER1,	TBL_BEGINRENDER2,
	C_EDITCOPYPLAYERNAME_CS, C_COPYSTRING_CS,
    C_SUB_MENUMODE, C_ADD_MENUMODE,
    C_READ_FILE, C_WRITE_FILE,
    C_COPY_DATA, C_COPY_DATA2,
    C_ENTER_UNIFORM_SELECT, C_EXIT_UNIFORM_SELECT,
    C_WRITE_DATA,
    C_READ_NAMES, C_READ_NAMES2,
    C_READ_DATA,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
DWORD codeArray[][CODELEN] = { 
  // PES2011 DEMO
  {
		0xff58f5,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0, 0,
        0, 0,
        0, 0,
        0,
        0, 0,
        0,
  },
  // PES2011
  {
		0x1071bb5,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xc7b313, 0xc7b1da,
        0x42d991, 0x1191201,
        0, 0,
        0,
        0, 0,
        0,
  },
  // PES2011 v1.01
  {
		0x1072115,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xc7b7f3, 0xc7b6ba,
        0x42d5b1, 0x1191751,
        0, 0, 
        0,
        0, 0,
        0,
  },
  // PES2011 v1.02
  {
		0x1072275,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xc7b883, 0xc7b74a,
        0x42d8c1, 0x11918b1,
        0, 0, 
        0,
        0, 0,
        0,
  },
  // PES2011 v1.03
  {
		0x1072285,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0xc7b893, 0xc7b75a,
        0x42d8c1, 0x11918f1,
        0, 0, 
        0,
        0, 0,
        0,
  },
  // PES2012 demo
  {
		0x1114c35,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0/*0x10e85f9*/, 0/*0x10e8582*/,
        0, 0,
        0, 0, 
        0,
        0, 0,
        0,
  },
  // PES2012 demo2
  {
		0x1128d95,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0x10fc759, 0x10fc6e2,
        0, 0,
        0, 0, 
        0,
        0, 0,
        0,
  },
  // PES2012
  {
		0x11801f5,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0x1153bb9, 0x1153b42,
        0, 0,
        0, 0, 
        0x12c3078,
        0x119cf2c, 0x12c3d7c,
        0xd3b1bb,
  },
  // PES2012 v1.01
  {
		0x1183615,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0x1156fd9, 0x1156f62,
        0, 0,
        0, 0, 
        0x12c6518,
        0x11a036c, 0x12c721c,
        0xd3cd1b,
  },
  // PES2012 v1.02
  {
		0, //0x1188e25,
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0x115c7e9, 0x115c772,
        0, 0,
        0, 0, 
        0x12c8098,
        0x11a5b7c, 0x12c8d9c,
        0xd3dbfb,
  },
  // PES2012 v1.03
  {
		0, 
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0, 0, //0x115c7e9, 0x115c772,
        0, 0,
        0, 0, 
        0x12c8d58, //0x12c8098,
        0x11a67fc, 0x12c9a5c, //0x11a5b7c, 0x12c8d9c,
        0xd3e3fb, //0xd3dbfb,
  },
  // PES2012 v1.06
  {
		0, 
		0, 0, 
		0, 0, 
		0, 0, 
        0, 0,
        0, 0,
        0, 0,
        0, 0, 
        0x12cb548, //0x12c8d58, 
        0x11a8e2c, 0x12cc24c, //0x11a67fc, 0x12c9a5c,
        0xd40e4b, //0xd3e3fb,
  },
};

#define DATALEN 13
enum {
	PLAYERDATA, ISREFEREEADDR,
	GENERALINFO, REPLAYGENERALINFO,
    MENU_MODE_IDX, MAIN_SCREEN_INDICATOR, INGAME_INDICATOR,
    NEXT_MATCH_DATA_PTR, CUP_MODE_PTR, EDIT_DATA_PTR,
    EDIT_DATA_SIZE, REPLAY_DATA_SIZE, BAL_DATA_SIZE,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0,0,0,0,0},
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        6902664, 4694064, 8389024,
    },
    // PES2011
    {
        0x1940b08, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        6902664, 4694064, 8389024,
    },
    // PES2011 v1.01
    {
        0x1940b60, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        6902664, 4694064, 8389024,
    },
    // PES2011 v1.02
    {
        0x1940b58, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        6902664, 4694064, 8389024,
    },
    // PES2011 v1.03
    {
        0x1940b5c, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        6902664, 4694064, 8389024,
    },
    // PES2012 demo
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
    // PES2012 demo2
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
    // PES2012
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
    // PES2012 v1.01
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
    // PES2012 v1.02
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
    // PES2012 v1.03
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
    // PES2012 v1.06
    {
        0, 0,
        0, 0,
        0, 0, 0,
        0, 0, 0,
        123, 456, 789,
    },
};

#define LTFPLEN 15
#define NOLTFPADDR {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
BYTE ltfpPatchArray[][LTFPLEN] = {
    // PES2011 DEMO
	NOLTFPADDR
    // PES2011 
	NOLTFPADDR
    // PES2011 v1.01
	NOLTFPADDR
    // PES2011 v1.02
	NOLTFPADDR
    // PES2011 v1.03
	NOLTFPADDR
    // PES2012 demo
	NOLTFPADDR
    // PES2012 demo2
	NOLTFPADDR
    // PES2012
	NOLTFPADDR
    // PES2012 v1.01
	NOLTFPADDR
    // PES2012 v1.02
	NOLTFPADDR
    // PES2012 v1.03
	NOLTFPADDR
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
BYTE ltfpPatch[LTFPLEN];
