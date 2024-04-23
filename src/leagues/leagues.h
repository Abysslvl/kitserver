#ifndef _LEAGUES_H
#define _LEAGUES_H

#define MODID 200
#define NAMELONG L"League Server " KITSERVER_VERSION_STRING
#define NAMESHORT L"LEAGUES"
#define DEFAULT_DEBUG 0

#define LEAGUES_MAX_SLOT 30
#define LEAGUES_LAST_BLOCK 22
#define LEAGUES_NEW_BLOCKS_SLOT 40

#include <Windows.h>
#include <string>

struct League
{
    DWORD relink;
    char* name;
    char* logo;
};

struct LeagueBlock
{
    DWORD numTeams;
    WORD* block;
};

#endif