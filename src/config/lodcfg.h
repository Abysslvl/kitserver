#ifndef _LODCFG_H_
#define _LODCFG_H_

#include <kconf.h>

#ifdef MYDLL_RELEASE_BUILD
#define LODCFG_WINDOW_TITLE "Kitserver Configuration Tool"
#else
#define LODCFG_WINDOW_TITLE "Kitserver Configuration Tool(debug build)"
#endif
#define CREDITS "Program info: " KITSERVER_VERSION_STRING " by Juce."

typedef struct _LCM {
    WORD homeTeamId;
    WORD awayTeamId;
    BYTE stadium;
    BYTE unknown1;
    BYTE timeOfDay;
    BYTE weather;
    BYTE season;
    BYTE effects;
    BYTE unknown2[6];
    BYTE unknown3[2];
    BYTE crowdStance;
    BYTE numSubs;
    BYTE homeCrowd;
    BYTE awayCrowd;
    BYTE unknown4[10];
} LCM;

#endif

