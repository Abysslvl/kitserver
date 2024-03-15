// lodmixer.h
#include <kconf.h>

#define MODID 101
#define NAMELONG L"LOD Mixer " KITSERVER_VERSION_STRING
#define NAMESHORT L"LODMIXER"
#define DEFAULT_DEBUG 0

#define BUFLEN 4096

typedef struct _DIMENSIONS {
    int width;
    int height;
    float aspectRatio;
} DIMENSIONS;

typedef struct _LOD {
    // player LODs
    float lodPlayersEntranceS1;  // entrace scene 
    float lodPlayersEntranceS2;  // ...
    float lodPlayersEntranceS3;  // ...
    float lodPlayersInplayS1;    // gameplay
    float lodPlayersInplayS2;    // ...
    float lodPlayersInplayS3;    // ...
    float lodPlayersMiscS1; // highlights/cinematics/cut-scenes/instant-replay
    float lodPlayersMiscS2;      // ...
    float lodPlayersMiscS3;      // ...
    float lodPlayersReplayS1;    // replay
    float lodPlayersReplayS2;    // ...
    float lodPlayersReplayS3;    // ...
    // referee LODs
    float lodRefInplay;          // gameplay
    float lodRefReplay;          // replay
    // active player LODs
    float lodActivePlayerCKs1;  // corner-kick
    float lodActivePlayerCKs2;
    float lodActivePlayerCKs3;
    float lodActivePlayerFKs1;  // free-kick
    float lodActivePlayerFKs2;
    float lodActivePlayerFKs3;  
} LOD;

typedef struct _LMCONFIG {
    DIMENSIONS screen;
    LOD lod;
    bool aspectRatioCorrectionEnabled;
    bool controllerCheckEnabled;
    bool lodCheck1;
    int pictureQuality;
} LMCONFIG;

#define DEFAULT_WIDTH 0
#define DEFAULT_HEIGHT 0
#define DEFAULT_ASPECT_RATIO 0.0f
#define DEFAULT_ASPECT_RATIO_CORRECTION_ENABLED false
#define DEFAULT_LOD_1 0.0f
#define DEFAULT_LOD_2 0.0f
#define DEFAULT_LOD_3 0.0f
#define DEFAULT_LOD_4 0.0f
#define DEFAULT_LOD_5 0.0f
#define DEFAULT_LOD_6 0.0f
#define DEFAULT_LOD_7 0.0f
#define DEFAULT_LOD_8 0.0f
#define DEFAULT_LOD_9 0.0f
#define DEFAULT_LOD_10 0.0f
#define DEFAULT_LOD_11 0.0f
#define DEFAULT_LOD_12 0.0f
#define DEFAULT_LOD_13 0.0f
#define DEFAULT_LOD_14 0.0f
#define DEFAULT_LOD_15 0.0f
#define DEFAULT_LOD_16 0.0f
#define DEFAULT_LOD_17 0.0f
#define DEFAULT_LOD_18 0.0f
#define DEFAULT_LOD_19 0.0f
#define DEFAULT_LOD_20 0.0f
#define GAME_DEFAULT_LOD_1 100.0f
#define GAME_DEFAULT_LOD_2 0.150f
#define GAME_DEFAULT_LOD_3 0.100f
#define GAME_DEFAULT_LOD_4 100.0f
#define GAME_DEFAULT_LOD_5 100.0f
#define GAME_DEFAULT_LOD_6 0.100f
#define GAME_DEFAULT_LOD_7 0.430f
#define GAME_DEFAULT_LOD_8 0.150f
#define GAME_DEFAULT_LOD_9 0.100f
#define GAME_DEFAULT_LOD_10 0.090f
#define GAME_DEFAULT_LOD_11 0.070f
#define GAME_DEFAULT_LOD_12 0.055f
#define GAME_DEFAULT_LOD_13 0.100f
#define GAME_DEFAULT_LOD_14 0.100f
#define GAME_DEFAULT_LOD_15 0.010f
#define GAME_DEFAULT_LOD_16 0.010f
#define GAME_DEFAULT_LOD_17 0.010f
#define GAME_DEFAULT_LOD_18 100.0f
#define GAME_DEFAULT_LOD_19 0.010f
#define GAME_DEFAULT_LOD_20 0.010f
#define DEFAULT_CONTROLLER_CHECK_ENABLED false
#define DEFAULT_LODCHECK1 true
#define DEFAULT_PICTURE_QUALITY -1

