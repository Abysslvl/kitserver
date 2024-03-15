// ADDRESSES for lodmixer.cpp
BYTE allowedGames[] = {
    //gvPES2011,
    //gvPES2011v101,
    //gvPES2011v102,
    //gvPES2011v103,
    gvPES2012demo,
    gvPES2012demo2,
    gvPES2012,
    gvPES2012v101,
    gvPES2012v102,
    gvPES2012v103,
    gvPES2012v106,
};

#define CODELEN 3
enum {
    C_SETTINGS_READ, C_QUALITY_CHECK, C_QUALITY_CHECK_2,
};

#define NOCODEADDR {0,0,0}
DWORD codeArray[][CODELEN] = { 
    // PES2011 demo
    NOCODEADDR,
    // PES2011 
    {
        0x1122e73, 0x112603a, 0x1125cf4,
    },
    // PES2011 v1.01 
    {
        0x11233d3, 0x112659a, 0x1126254,
    },
    // PES2011 v1.02 
    {
        0x1123533, 0x11266fa, 0x11263b4,
    },
    // PES2011 v1.03 
    {
        0x1123573, 0x112673a, 0x11263f4,
    },
    // PES2012 demo
    {
        0x11e0f33, 0x11e404f, 0x11e3dd1,
    },
    // PES2012 demo2
    {
        0x11f4ef3, 0x11f801f, 0x11f7da1,
    },
    // PES2012
    {
        0x124d0e3, 0x12502ef, 0x1250071,
    },
    // PES2012 v1.01
    {
        0x1250583, 0x125378f, 0x1253511,
    },
    // PES2012 v1.02
    {
        0x1255d93, 0x1258f9f, 0x1258d21,
    },
    // PES2012 v1.03
    {
        0x1256a13, 0x1259c1f, 0x12599a1,
        /**
        0x503fcb, //stadium/weather options
        //next call: push 0 for second param
        **/
    },
    // PES2012 v1.06
    {
        0x1259203, 0x125c40f, 0x125c191,
    },
};

#define DATALEN 9
enum {
    SCREEN_WIDTH, SCREEN_HEIGHT, WIDESCREEN_FLAG,
    RATIO_4on3, RATIO_16on9,
    LOD_PLAYERS_TABLE1, LOD_PLAYERS_TABLE2, 
    LOD_REF_TABLE1, LOD_REF_TABLE2,
};

#define NODATAADDR {0,0,0,0,0,0,0,0,0}
DWORD dataArray[][DATALEN] = {
    // PES2011 demo
    NODATAADDR,
    // PES2011 
    {
        0x1940370, 0x1940374, 0x1940378,
        0x143df84, 0x1440da0,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788,
    },
    // PES2011 v1.01
    {
        0x19403c8, 0x19403cc, 0x19403d0,
        0x143e304, 0x1441120,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
    // PES2011 v1.02
    {
        0x19403c0, 0x19403c4, 0x19403c8,
        0x143e30c, 0x1441128,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
    // PES2011 v1.03
    {
        0x19403c0, 0x19403c4, 0x19403c8,
        0x143e30c, 0x1441128,
        0x15a2dd0, 0x15a2e18,
        0x1535760, 0x1535788, 
    },
    // PES2012 demo
    {
        0x199a388, 0x199a38c, 0x199a390,
        0x14efe98, 0x14f040c,
        0x165fae8, 0x165fb30,
        0x165f0e4, 0x165f10c,
    },
    // PES2012 demo2
    {
        0x19b09c0, 0x19b09c4, 0x19b09c8,
        0x1505828, 0x1505dcc,
        0x1675f80, 0x1675fc8,
        0x167557c, 0x16755a4,
    },
    // PES2012
    {
        0x1a15090, 0x1a15094, 0x1a15098,
        0x1563b68, 0x156410c,
        0x16d5148, 0x16d5190,
        0x16d4744, 0x16d476c,
    },
    // PES2012 v1.01
    {
        0x1a26258, 0x1a2625c, 0x1a26260,
        0x1574878, 0x1574e1c,
        0x16e6260, 0x16e62a8,
        0x16e585c, 0x16e5884,
    },
    // PES2012 v1.02
    {
        0x1a28268, 0x1a2826c, 0x1a28270,
        0x1576ac8, 0x157706c,
        0x16e8260, 0x16e82a8,
        0x16e785c, 0x16e7884,
    },
    // PES2012 v1.03
    {
        0x1a2a280, 0x1a2a284, 0x1a2a288,
        0x1578ab8, 0x157905c,
        0x16ea260, 0x16ea2a8,
        0x16e985c, 0x16e9884,
    },
    // PES2012 v1.06
    {
        0x1a28a70, 0x1a28a74, 0x1a28a78,
        0x1576b68, 0x157710c,
        0x16e8a18, 0x16e8a60,
        0x16e8014, 0x16e803c,
    },
};

DWORD code[CODELEN];
DWORD dta[DATALEN];
