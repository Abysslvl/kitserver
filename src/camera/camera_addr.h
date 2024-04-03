// Addresses for camera settings
#ifndef _CAMERA_ADDR_H
#define _CAMERA_ADDR_H

#include <windows.h>
#include <manage.h>

constexpr BYTE allowedGames[] =
{
    gvPES2013v103,
    gvPES2013v104
};

#define CODELEN 1
enum
{
    DUMMY
};

#define NOCODEADDR {0},
constexpr DWORD codeArray[][CODELEN] =
{
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
    NOCODEADDR
};

#define DATALEN 2
enum
{
    CAMERA_SETTINGS_TABLE, NUM_CAMS
};

#define NODATAADDR {0, 0},
DWORD dataArray[][DATALEN] =
{
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
        0x0, 18
    },
    // PES2013 v1.04
    {
        0x14ecea8, 18
    }
};

DWORD code[CODELEN];
DWORD dta[DATALEN];

#endif
