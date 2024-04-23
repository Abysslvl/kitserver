#ifndef _LEAGUES_ADDR_H
#define _LEAGUES_ADDR_H

#include <Windows.h>
#include "manage.h"

// ADDRESSES for kserv.cpp
BYTE allowedGames[] = {
    gvPES2013v104
};

#define CODELEN 9
enum {
    C_READ_CLUBS, C_READ_NATIONALS, C_MAKE_LEAGUE, C_READ_RELINK, C_READ_LOGO,
    C_READ_FLAG, C_BEFORE_READ_FLAG,
    C_READ_TOTAL_BLOCKS, C_READ_BLOCK,
};

#define NOCODEADDR {0,0,0,0,0,0,0,0,0},
constexpr DWORD codeArray[][CODELEN] = {
    // PES2013 demo
    NOCODEADDR
    // PES2013 v1.00
    NOCODEADDR
    // PES2013 v1.01
    NOCODEADDR
    // PES2013 v1.02
    NOCODEADDR
    // PES2013 v1.03
    NOCODEADDR
    // PES2013 v1.04 
    {
        0x531FC2, 0x531B41, 0x531850, 0x531880, 0x531893,
        0x52F7F0, 0x52F856,
        0x1186998, 0x52CA1C,
    }
};

#define DATALEN 4
enum {
    LEAGUES_RELINK_TABLE, LEAGUES_LOGO_TABLE, 
    LEAGUES_TEX_TABLE, LEAGUES_FLAG_TABLE,
};

#define NODATAADDR {0,0,0,0},
constexpr DWORD dataArray[][DATALEN] = {
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
    {
       0x140A280, 0x140A3C0, 0x15E0200, 0x140A420, 
    }
};

DWORD code[CODELEN];
DWORD dta[DATALEN];

#endif