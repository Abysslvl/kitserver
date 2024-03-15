#ifndef __JUCE_GDB__
#define __JUCE_GDB__

#include <windows.h>
#include <map>
#include <hash_map>
#include <string>

using namespace std;
#if _CPPLIB_VER >= 503
using namespace stdext;
#endif

// attribute definition flags (bits)
#define SHIRT_NUMBER                 0x00000001 
#define SOCKS_COLOR                  0x00000002 
#define SHORTS_NUMBER                0x00000004 
#define COLLAR                       0x00000008 
#define MODEL                        0x00000010 
#define SLEEVE_PATCH_LEFT_POS_SHORT  0x00000020 
#define SHORTS_NUMBER_LOCATION       0x00000040 
#define NUMBER_TYPE                  0x00000080
#define NAME_TYPE                    0x00000100
#define NAME_SHAPE                   0x00000200
#define NUMBERS_FILE                 0x00000400
#define SECOND_COLOR                 0x00000800
#define NAME_SHOW                    0x00001000
#define ICON_TYPE                    0x00002000
#define MAIN_COLOR                   0x00004000
#define MASK_FILE                    0x00008000
#define KITDESCRIPTION               0x00010000
#define FRONT_NUMBER_SHOW            0x00020000
#define SHORTS_MAIN_COLOR            0x00040000
#define SHORTS_NUMBER_Y              0x00080000
#define SHORTS_NUMBER_X              0x00100000
#define SHORTS_NUMBER_SIZE           0x00200000
#define FRONT_NUMBER_Y               0x00400000
#define FRONT_NUMBER_X               0x00800000
#define FRONT_NUMBER_SIZE            0x01000000
#define SLEEVE_PATCH_LEFT_POS_LONG   0x02000000
#define SLEEVE_PATCH_RIGHT_POS_SHORT 0x04000000
#define SLEEVE_PATCH_RIGHT_POS_LONG  0x08000000
#define NAME_SIZE                    0x10000000
#define NAME_Y                       0x20000000
#define NUMBER_Y                     0x40000000
#define NUMBER_SIZE                  0x80000000

// GDB data structures
///////////////////////////////

class RGBAColor {
public:
    BYTE r;
    BYTE g;
    BYTE b;
    BYTE a;
};

class Kit {
public:
    wstring foldername;
    wstring description;
    BYTE collar;
    WORD model;
    BYTE frontNumberShow;
    BYTE shortsNumberLocation;
    BYTE numberType;
    BYTE nameShow;
    BYTE nameType;
    BYTE nameShape;
    BYTE logoLocation;
    RGBAColor mainColor;
    RGBAColor secondColor;
    RGBAColor shortsFirstColor;
    RGBAColor socksColor;
    //wstring maskFile;
    //wstring shirtFolder;
    //wstring shortsFolder;
    //wstring socksFolder;
    //wstring overlayFile;
    BYTE frontNumberY;
    BYTE frontNumberX;
    BYTE frontNumberSize;
    BYTE sleevePatchRightPosShort;
    BYTE sleevePatchRightPosLong;
    BYTE sleevePatchLeftPosShort;
    BYTE sleevePatchLeftPosLong;
    BYTE shortsNumberSize;
    BYTE shortsNumberY;
    BYTE shortsNumberX;
    BYTE numberY;
    BYTE numberSize;
    BYTE nameY;
    BYTE nameSize;
    DWORD attDefined;
    bool configLoaded;
    WORD slot;
    BYTE iconType;

    Kit() : attDefined(0), configLoaded(false), slot(0xffff) {}
};

class KitCollection {
public:
    wstring foldername;
    map<wstring,Kit> players;
    map<wstring,Kit> goalkeepers;
    map<wstring,Kit>::iterator pa;
    map<wstring,Kit>::iterator pb;
    map<wstring,Kit>::iterator ga;
    map<wstring,Kit>::iterator gb;
    map<wstring,Kit>::iterator euro_pa;
    map<wstring,Kit>::iterator euro_pb;
    map<wstring,Kit>::iterator euro_ga;
    map<wstring,Kit>::iterator euro_gb;
    bool loaded;
    bool disabled;

    KitCollection(const wstring& fname) : 
        foldername(fname), 
        loaded(false),
        disabled(false),
        pa(players.end()),
        pb(players.end()),
        ga(players.end()),
        gb(players.end()),
        euro_pa(players.end()),
        euro_pb(players.end()),
        euro_ga(players.end()),
        euro_gb(players.end())
    {}
};

class GDB {
public:
    wstring dir;
    wstring uniMapFile;
    hash_map<WORD,KitCollection> uni;
    KitCollection dummyHome;
    KitCollection dummyAway;
    bool readConfigs;

    GDB(const wstring& gdir, const wstring& unimap, bool rc=true) : 
        dir(gdir), 
        uniMapFile(unimap),
        dummyHome(L""), 
        dummyAway(L""), 
        readConfigs(rc)
    { this->load(); }
    void loadConfig(Kit& kit);

private:
    void load();
    void findKitsForTeam(WORD teamId);
    void fillKitCollection(KitCollection& col, int kitType, bool extra=false);
};

#endif
