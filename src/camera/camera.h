#ifndef _CAMERA_H
#define _CAMERA_H

#include <kconf.h>
#include <windows.h>

#define MODID 102
#define NAMELONG L"Camera Settings " KITSERVER_VERSION_STRING
#define NAMESHORT L"CAMERA"
#define DEFAULT_DEBUG 0

struct camera
{
    float range;
    float zoomNear;
    float height;
    float unknow1;
    float zoomFar;
    float unknow2;
    float unknow3;
    float cameraAngle;
    float angleModulator;
    DWORD unknow4[0xa];
}; // sizeof = 0x4c (76 bytes)

constexpr const char* camsNames[18] =
{
    "close",
    "mid.range",
    "long",
    "wide",
    "pitch.side",
    "blimp",
    "wide.broadcast",
    "long.broadcast",
    "live.broadcast",
    "close.broadcast",
    "hidden.1",
    "hidden.2",
    "hidden.3",
    "vertical.wide",
    "player",
    "hidden.4",
    "front",
    "vertical.far",
};

constexpr bool ableCams[18] =
{
    false,
    true,
    true,
    true,
    true,
    true,
    false,
    false,
    true,
    false,
    false,
    false,
    false,
    true,
    true,
    false,
    false,
    false,
};

#endif