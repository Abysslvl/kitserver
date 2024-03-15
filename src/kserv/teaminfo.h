// teaminfo.h

#ifndef _TEAMINFO_H_
#define _TEAMINFO_H_

#pragma pack(push, 1)

typedef WORD KCOLOR;

typedef struct _KIT_INFO {
    DWORD unknown1[3];
    KCOLOR mainColor;
    KCOLOR editShirtColors[2];
    KCOLOR secondColor;
    KCOLOR editShirtColor5;
    KCOLOR shortsFirstColor;
    KCOLOR editKitColors[3];
    KCOLOR socksColor;
    KCOLOR editKitColors2[5];
    BYTE unknown2[16];
    BYTE collar;
    BYTE editKitStyles[10];
    BYTE nameShow;
    BYTE unknown22;
    BYTE nameShape;
    BYTE unknown3;
    BYTE frontNumberShow;
    BYTE shortsNumberPosition;
    BYTE unknown4;
    BYTE unknown5;
    BYTE sleevePatchRightPosLong;
    BYTE sleevePatchRightPosShort;
    BYTE sleevePatchLeftPosLong;
    BYTE sleevePatchLeftPosShort;
    BYTE unknown6;
    BYTE numberY;
    BYTE numberSize;
    BYTE frontNumberY;
    BYTE frontNumberX;
    BYTE frontNumberSize;
    BYTE shortsNumberY;
    BYTE shortsNumberX;
    BYTE shortsNumberSize;
    BYTE nameY;
    BYTE nameSize;
    BYTE flagPosition;
    BYTE unknown7[3];
    WORD unknown8;
    WORD model;
    BYTE techfit;
    BYTE iconType;
} KIT_INFO; // size = 0x66

const char* _kitInfoDefault =
    "\xff\xff\xff\xff\xff\xff"
    "\xff\xff\xff\xff\xff\xff\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80"
    "\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80"
    "\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80\x00\x80"
    "\x00\x80\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

typedef struct _TEAM_KIT_INFO
{
    WORD id;
    WORD slot;
    KIT_INFO ga;
    KIT_INFO pa;
    KIT_INFO pb;
} TEAM_KIT_INFO; // size = 0x66*3+4

typedef struct _TEAM_MATCH_DATA_INFO
{
    WORD teamIdSpecial;
    WORD teamId;
    //BYTE unknown2[0x2d30];
    //BYTE kitSelection;
    //BYTE unknown3[11];
    //TEAM_KIT_INFO tki;
} TEAM_MATCH_DATA_INFO;

typedef struct _NEXT_MATCH_DATA_INFO
{
    TEAM_MATCH_DATA_INFO* home;
    TEAM_MATCH_DATA_INFO* away;
} NEXT_MATCH_DATA_INFO;
#pragma pack(pop)

#endif
