// player.h

#ifndef _PLAYER_H
#define _PLAYER_H

#define MAX_PLAYERS 0x2f80

#define SPECIAL_FACE 0x20
#define SPECIAL_HAIR 0x40
#define EDITED_FACEHAIR 0x10
#define IMPORTED_FACE 0x80

#pragma pack(push, 1)

typedef struct _PLAYER_INFO
{
    DWORD id;
    BYTE unknown1[0x2e];
    char name[0x2e];
    char name2[0x10];
    DWORD id_again;
    BYTE unknown2[0x30];
    BYTE unknown3[3];
    BYTE specialHair;
    DWORD faceHairBits;
    BYTE unknown4[0x2c];
    WORD nationality;
    BYTE specialFace;
    BYTE unknown5;
    DWORD unknown6;
    WORD unknown7;
    WORD index;

} PLAYER_INFO;  //sizeof = 0xe4

typedef struct _PLAYER_DETAILS
{
    BYTE unknown3[3];
    BYTE specialHair;
    DWORD faceHairBits;
    BYTE unknown4[0x2c];
    WORD nationality;
    BYTE specialFace;
    BYTE unknown5;
    DWORD unknown6;
    WORD unknown7;
    WORD index;

} PLAYER_DETAILS; 

#pragma pack(pop)

/*****************************************
 * Face/Hair bits:
 *
 * 00-10 : (11) hair id (bin-number - 3705 + 1)
 * 11-14 :  (4) hair flags
 * 15-25 : (11) face id (bin-number - 131 + 1)
 * 26-29 :  (4) face flags
 * 30-31 :  (2) unknown
 *
 */

const DWORD FIRST_HAIR_BIN = 3705;
const DWORD FIRST_FACE_BIN = 131;

#define CLEAR_HAIR_MASK  0xfffff800
#define CLEAR_HAIR_FLAGS 0xffff87ff
#define CLEAR_FACE_MASK  0xffc07fff
#define CLEAR_FACE_FLAGS 0xc3ffffff

#endif
