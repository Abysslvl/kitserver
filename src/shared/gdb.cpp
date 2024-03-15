#define UNICODE
#define THISMOD &k_kserv

#include <stdio.h>
#include <string>
#include "gdb.h"
#include "configs.h"
#include "configs.hpp"
#include "utf8.h"
#include "log.h"

extern KMOD k_kserv;

#ifdef MYDLL_RELEASE_BUILD
#define GDB_DEBUG(f,x)
#define GDB_DEBUG_OPEN(f,dir)
#define GDB_DEBUG_CLOSE(f)

#else
#define GDB_DEBUG1(f,x) if (f) {\
    swprintf x;\
    char a_str[BUFLEN];\
    ZeroMemory(a_str, BUFLEN);\
    Utf8::fUnicodeToUtf8((BYTE*)a_str,slog);\
    fputs(a_str,f);\
}
#define GDB_DEBUG(f,x)
#define GDB_DEBUG_OPEN(f,dir) {\
    f = _wfopen((dir + L"GDB.debug.log").c_str(),L"wt");\
}
#define GDB_DEBUG_CLOSE(f) if (f) fclose(f)
FILE* wlog;
wchar_t slog[WBUFLEN];
#endif

#define PLAYERS 0
#define GOALKEEPERS 1
#define EURO_PLAYERS 2
#define EURO_GOALKEEPERS 3

enum {
    ATT_MODEL,
    ATT_COLLAR,
    ATT_FRONT_NUMBER_SHOW,
    ATT_SHORTS_NUMBER_LOCATION,
    ATT_NAME_SHOW,
    ATT_NAME_SHAPE,
    ATT_MAIN_COLOR,
    ATT_SECOND_COLOR,
    ATT_SHORTS_COLOR,
    ATT_SOCKS_COLOR,
    ATT_ICON_TYPE,
    ATT_DESCRIPTION,
    ATT_FRONT_NUMBER_Y,
    ATT_FRONT_NUMBER_X,
    ATT_FRONT_NUMBER_SIZE,
    ATT_SLEEVE_PATCH_LEFT_POS_SHORT,
    ATT_SLEEVE_PATCH_LEFT_POS_LONG,
    ATT_SLEEVE_PATCH_RIGHT_POS_SHORT,
    ATT_SLEEVE_PATCH_RIGHT_POS_LONG,
    ATT_SHORTS_NUMBER_SIZE,
    ATT_SHORTS_NUMBER_Y,
    ATT_SHORTS_NUMBER_X,
    ATT_NUMBER_Y,
    ATT_NUMBER_SIZE,
    ATT_NAME_Y,
    ATT_NAME_SIZE,
};

class kattr_data
{
public:
    Kit& kit;
    int attr;

    kattr_data(Kit& k, int a) : kit(k), attr(a) {}
};

// functions
//////////////////////////////////////////

static bool ParseColor(const wchar_t* str, RGBAColor* color);
static void kitConfig(char* pName, const void* pValue, DWORD a);
static bool equals(const void* a, const void* b);
static void string_strip(wstring& s);

static bool equals(const void* a, const void* b)
{
    wstring sa((wchar_t*)a);
    wstring sb((wchar_t*)b);
    string_strip(sa);
    string_strip(sb);
    return sa == sb;
}

static void string_strip(wstring& s)
{
    static const wchar_t* empties = L" \t\n\r";
    int e = s.find_last_not_of(empties);
    s.erase(e + 1);
    int b = s.find_first_not_of(empties);
    s.erase(0,b);
}

/**
 * Allocate and initialize the GDB structure, read the "map.txt" file
 * but don't look for kit folders themselves.
 */
void GDB::load()
{
	GDB_DEBUG_OPEN(wlog,this->dir);
	GDB_DEBUG(wlog,(slog,L"Loading GDB...\n"));

    // process kit map file
    hash_map<WORD,wstring> mapFile;
    if (!readMap(this->uniMapFile.c_str(), mapFile))
    {
        GDB_DEBUG(wlog,(slog,L"Unable to find uni-map: %s\n",mapFile));
        LOG(L"Couldn't open uni-map for reading: {%s}",
            this->uniMapFile.c_str());
    }

    for (hash_map<WORD,wstring>::iterator it = mapFile.begin(); it != mapFile.end(); it++)
    {
        //LOG(L"GDB map: %d --> {%s}", it->first, it->second.c_str());
        KitCollection kitCol(it->second);

        // strip off leading and trailing spaces
        string_strip(kitCol.foldername);

        // strip off quotes, if present
        if (kitCol.foldername[0]=='"' || kitCol.foldername[0]=='\'')
            kitCol.foldername.erase(0,1);
        int last = kitCol.foldername.length()-1;
        if (kitCol.foldername[last]=='"' || kitCol.foldername[last]=='\'')
            kitCol.foldername.erase(last);

        if (kitCol.foldername.empty())
            continue;

        // check directory existence
        wstring dirName(this->dir);
        dirName += L"GDB\\uni\\";
        dirName += kitCol.foldername;
        if (GetFileAttributes(dirName.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            LOG(L"ERROR: Unable to find or read folder for team %d (%s). Skipping it.", it->first, dirName.c_str());
            continue;
        }

        GDB_DEBUG(wlog,(slog,L"teamId = {%d}, foldername = {%s}\n", 
                    it->first, kitCol.foldername.c_str()));

        // store in the "uni" map
        this->uni.insert(pair<WORD,KitCollection>(it->first,kitCol));

        // find kits for this team
        this->findKitsForTeam(it->first);
    }

    // create two dummy kit collections: for use with AFS kits
    Kit dummyKit;
    dummyKit.configLoaded = true;

    this->dummyHome.players.insert(pair<wstring,Kit>(L"pa",dummyKit));
    this->dummyHome.players.insert(pair<wstring,Kit>(L"pb",dummyKit));
    this->dummyHome.goalkeepers.insert(pair<wstring,Kit>(L"ga",dummyKit));
    this->dummyHome.goalkeepers.insert(pair<wstring,Kit>(L"gb",dummyKit));
    this->dummyHome.loaded = true;

    this->dummyAway.players.insert(pair<wstring,Kit>(L"pa",dummyKit));
    this->dummyAway.players.insert(pair<wstring,Kit>(L"pb",dummyKit));
    this->dummyAway.goalkeepers.insert(pair<wstring,Kit>(L"ga",dummyKit));
    this->dummyAway.goalkeepers.insert(pair<wstring,Kit>(L"gb",dummyKit));
    this->dummyAway.loaded = true;

	GDB_DEBUG(wlog,(slog,L"Loading GDB complete.\n"));
    GDB_DEBUG_CLOSE(wlog);
}

/**
 * Enumerate all kits in this team folder.
 * and for each one parse the "config.txt" file.
 */
void GDB::fillKitCollection(KitCollection& col, int kitType, bool extra)
{
	WIN32_FIND_DATA fData;
    wstring pattern(this->dir);

	if (kitType == PLAYERS) {
		pattern += L"GDB\\uni\\" + col.foldername + L"\\p*";
    } else if (kitType == GOALKEEPERS) { 
		pattern += L"GDB\\uni\\" + col.foldername + L"\\g*";
    } else if (kitType == EURO_PLAYERS) {
		pattern += L"GDB\\uni\\" + col.foldername + L"\\euro-p*";
    } else if (kitType == EURO_GOALKEEPERS) {
		pattern += L"GDB\\uni\\" + col.foldername + L"\\euro-g*";
    }

    // pre-insert pa/pb kits
    if (kitType == PLAYERS)
    {
        Kit kitPA;
        kitPA.foldername = L"GDB\\uni\\" + col.foldername + L"\\pa";
        // read and parse the config.txt
        kitPA.attDefined = 0;
        if (this->readConfigs)
            this->loadConfig(kitPA);

        Kit kitPB;
        kitPB.foldername = L"GDB\\uni\\" + col.foldername + L"\\pb";
        // read and parse the config.txt
        kitPB.attDefined = 0;
        if (this->readConfigs)
            this->loadConfig(kitPB);

        col.players.insert(pair<wstring,Kit>(L"pa",kitPA));
        col.players.insert(pair<wstring,Kit>(L"pb",kitPB));
    }

	GDB_DEBUG(wlog,(slog, L"pattern = {%s}\n",pattern));

	HANDLE hff = FindFirstFile(pattern.c_str(), &fData);
	if (hff == INVALID_HANDLE_VALUE) 
	{
		// none found.
        if (!extra) {
            LOG(L"WARNING: no kit folders found for search pattern: {%s}", 
                pattern.c_str());
        }
		return;
	}
	while(true)
	{
        GDB_DEBUG(wlog,(slog,L"found: {%s}\n",fData.cFileName));
        //LOG(L"found: {%s}",fData.cFileName);
        // check if this is a directory
        if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            wstring key(fData.cFileName);
            if (key!=L"pa" && key!=L"pb")
            {
                Kit kit;
                kit.foldername = L"GDB\\uni\\" + col.foldername + L"\\" + wstring(fData.cFileName);

                // read and parse the config.txt
                kit.attDefined = 0;

                if (this->readConfigs)
                    this->loadConfig(kit);

                // make sure extra kits follow *a,*b
                if (key!=L"pa" && key!=L"pb" && key!=L"ga" && key!=L"gb")
                {
                    key = L"x-";
                    key += fData.cFileName;
                    //LOG(L"kit-key: %s", key.c_str());
                }

                // insert kit object into KitCollection map
                if (kitType == PLAYERS || kitType == EURO_PLAYERS)
                    col.players.insert(pair<wstring,Kit>(key,kit));
                else if (kitType == GOALKEEPERS || kitType == EURO_GOALKEEPERS)
                    col.goalkeepers.insert(pair<wstring,Kit>(key,kit));
            }
		}

		// proceed to next file
		if (!FindNextFile(hff, &fData)) break;
	}

	FindClose(hff);
}

/**
 * Enumerate all kits in this team folder.
 * and for each one parse the "config.txt" file.
 */
void GDB::findKitsForTeam(WORD teamId)
{
    hash_map<WORD,KitCollection>::iterator it = this->uni.find(teamId);
    if (it != this->uni.end() && !it->second.loaded)
    {
        // players
        this->fillKitCollection(it->second, PLAYERS);
        this->fillKitCollection(it->second, EURO_PLAYERS, true);
        // goalkeepers
        this->fillKitCollection(it->second, GOALKEEPERS);
        this->fillKitCollection(it->second, EURO_GOALKEEPERS, true);

        // initialize iterators to main kits
        it->second.pa = it->second.players.find(L"pa");
        it->second.pb = it->second.players.find(L"pb");
        it->second.ga = it->second.goalkeepers.find(L"ga");
        it->second.gb = it->second.goalkeepers.find(L"gb");
        /*
        if (it->second.pa != it->second.players.end())
            LOG(L"@found pa");
        if (it->second.pb != it->second.players.end())
            LOG(L"@found pb");
        if (it->second.ga != it->second.goalkeepers.end())
            LOG(L"@found ga");
        if (it->second.gb != it->second.goalkeepers.end())
            LOG(L"@found gb");
        */

        // initialize iterators to euro kits
        it->second.euro_pa = it->second.players.find(L"x-euro-pa");
        it->second.euro_pb = it->second.players.find(L"x-euro-pb");
        it->second.euro_ga = it->second.goalkeepers.find(L"x-euro-ga");
        it->second.euro_gb = it->second.goalkeepers.find(L"x-euro-gb");
        /*
        if (it->second.euro_pa != it->second.players.end())
            LOG(L"@found euro_pa");
        if (it->second.euro_pb != it->second.players.end())
            LOG(L"@found euro_pb");
        if (it->second.euro_ga != it->second.goalkeepers.end())
            LOG(L"@found euro_ga");
        if (it->second.euro_gb != it->second.goalkeepers.end())
            LOG(L"@found euro_gb");
        */

        // mark kit collection as loaded
        it->second.loaded = true;
    }
}

/**
 * Read and parse the config.txt for the given kit.
 */
void GDB::loadConfig(Kit& kit)
{
    if (kit.configLoaded) return;

    //LOG(L"path: {%s}",(this->dir + kit.foldername + L"\\config.txt").c_str());
    if (readConfig((this->dir + kit.foldername + L"\\config.txt").c_str()))
    {
        _getConfig("", "model", DT_DWORD, (DWORD)&kattr_data(kit,ATT_MODEL), kitConfig);
        _getConfig("", "collar", DT_DWORD, (DWORD)&kattr_data(kit,ATT_COLLAR), kitConfig);
        _getConfig("", "front.number.show", DT_DWORD, (DWORD)&kattr_data(kit,ATT_FRONT_NUMBER_SHOW), kitConfig);
        _getConfig("", "shorts.number.location", DT_STRING, (DWORD)&kattr_data(kit,ATT_SHORTS_NUMBER_LOCATION), kitConfig);
        _getConfig("", "name.show", DT_DWORD, (DWORD)&kattr_data(kit,ATT_NAME_SHOW), kitConfig);
        _getConfig("", "name.shape", DT_STRING, (DWORD)&kattr_data(kit,ATT_NAME_SHAPE), kitConfig);
        _getConfig("", "main.color", DT_STRING, (DWORD)&kattr_data(kit,ATT_MAIN_COLOR), kitConfig);
        _getConfig("", "radar.color", DT_STRING, (DWORD)&kattr_data(kit,ATT_MAIN_COLOR), kitConfig); // for backward compatibility
        _getConfig("", "shorts.color", DT_STRING, (DWORD)&kattr_data(kit,ATT_SHORTS_COLOR), kitConfig);
        _getConfig("", "second.color", DT_STRING, (DWORD)&kattr_data(kit,ATT_SECOND_COLOR), kitConfig);
        _getConfig("", "socks.color", DT_STRING, (DWORD)&kattr_data(kit,ATT_SOCKS_COLOR), kitConfig);
        _getConfig("", "icon.type", DT_DWORD, (DWORD)&kattr_data(kit,ATT_ICON_TYPE), kitConfig);
        _getConfig("", "description", DT_STRING, (DWORD)&kattr_data(kit,ATT_DESCRIPTION), kitConfig);
        _getConfig("", "front.number.y", DT_DWORD, (DWORD)&kattr_data(kit,ATT_FRONT_NUMBER_Y), kitConfig);
        _getConfig("", "front.number.x", DT_DWORD, (DWORD)&kattr_data(kit,ATT_FRONT_NUMBER_X), kitConfig);
        _getConfig("", "front.number.size", DT_DWORD, (DWORD)&kattr_data(kit,ATT_FRONT_NUMBER_SIZE), kitConfig);
        _getConfig("", "sleeve.patch.left.pos.short", DT_DWORD, (DWORD)&kattr_data(kit,ATT_SLEEVE_PATCH_LEFT_POS_SHORT), kitConfig);
        _getConfig("", "sleeve.patch.left.pos.long", DT_DWORD, (DWORD)&kattr_data(kit,ATT_SLEEVE_PATCH_LEFT_POS_LONG), kitConfig);
        _getConfig("", "sleeve.patch.right.pos.short", DT_DWORD, (DWORD)&kattr_data(kit,ATT_SLEEVE_PATCH_RIGHT_POS_SHORT), kitConfig);
        _getConfig("", "sleeve.patch.right.pos.long", DT_DWORD, (DWORD)&kattr_data(kit,ATT_SLEEVE_PATCH_RIGHT_POS_LONG), kitConfig);
        _getConfig("", "shorts.number.size", DT_DWORD, (DWORD)&kattr_data(kit,ATT_SHORTS_NUMBER_SIZE), kitConfig);
        _getConfig("", "shorts.number.y", DT_DWORD, (DWORD)&kattr_data(kit,ATT_SHORTS_NUMBER_Y), kitConfig);
        _getConfig("", "shorts.number.x", DT_DWORD, (DWORD)&kattr_data(kit,ATT_SHORTS_NUMBER_X), kitConfig);
        _getConfig("", "number.y", DT_DWORD, (DWORD)&kattr_data(kit,ATT_NUMBER_Y), kitConfig);
        _getConfig("", "number.size", DT_DWORD, (DWORD)&kattr_data(kit,ATT_NUMBER_SIZE), kitConfig);
        _getConfig("", "name.y", DT_DWORD, (DWORD)&kattr_data(kit,ATT_NAME_Y), kitConfig);
        _getConfig("", "name.size", DT_DWORD, (DWORD)&kattr_data(kit,ATT_NAME_SIZE), kitConfig);

        LOG(L"config.txt loaded for {%s}",kit.foldername.c_str());
    }
    else
    {
        GDB_DEBUG(wlog, (slog, L"Unable to find config.txt for %s\n", kit.foldername.c_str()));
        LOG(L"ERROR: unable to read config.txt for {%s}. Error code = %d",kit.foldername.c_str(), errno);
    }

    kit.configLoaded = true;
}

/**
 * Callback function for reading of config.txt
 */
static void kitConfig(char* pName, const void* pValue, DWORD a)
{
    kattr_data* kd = (kattr_data*)a;
    if (!kd) return;

    switch (kd->attr)
    {
        case ATT_MODEL:
            kd->kit.model = *(DWORD*)pValue;
            kd->kit.attDefined |= MODEL;
            GDB_DEBUG(wlog,(slog,L"model = %d\n",kd->kit.model));
            break;

        case ATT_COLLAR:
            kd->kit.collar = *(DWORD*)pValue;
            kd->kit.attDefined |= COLLAR;
            GDB_DEBUG(wlog,(slog,L"collar = %d\n",kd->kit.collar));
            break;

        case ATT_FRONT_NUMBER_SHOW:
            kd->kit.frontNumberShow = (*(DWORD*)pValue == 1);
            kd->kit.attDefined |= FRONT_NUMBER_SHOW;
            GDB_DEBUG(wlog,(slog,L"frontNumberShow = %d\n",
                        kd->kit.frontNumberShow));
            break;

        case ATT_SHORTS_NUMBER_LOCATION:
            if (equals(pValue,L"off"))
                kd->kit.shortsNumberLocation = 0;
            else if (equals(pValue,L"left"))
                kd->kit.shortsNumberLocation = 1;
            else if (equals(pValue,L"right"))
                kd->kit.shortsNumberLocation = 2;
            kd->kit.attDefined |= SHORTS_NUMBER_LOCATION;
            GDB_DEBUG(wlog,(slog,L"shortsNumberLocation = %d\n",kd->kit.shortsNumberLocation));
            break;

        case ATT_NAME_SHOW:
            kd->kit.nameShow = (*(DWORD*)pValue == 1);
            kd->kit.attDefined |= NAME_SHOW;
            GDB_DEBUG(wlog,(slog,L"nameShow = %d\n",kd->kit.nameShow));
            break;

        case ATT_NAME_SHAPE:
            if (equals(pValue, L"type1"))
                kd->kit.nameShape = 0;
            else if (equals(pValue, L"type2"))
                kd->kit.nameShape = 1;
            else if (equals(pValue, L"type3"))
                kd->kit.nameShape = 2;
            else if (equals(pValue, L"type4"))
                kd->kit.nameShape = 3;
            kd->kit.attDefined |= NAME_SHAPE;
            GDB_DEBUG(wlog,(slog,L"nameShape = %d\n",kd->kit.nameShape));
            break;

            
        case ATT_MAIN_COLOR:
            if (ParseColor((wchar_t*)pValue, &kd->kit.mainColor))
                kd->kit.attDefined |= MAIN_COLOR;
            GDB_DEBUG(wlog,(slog,L"mainColor = %02x%02x%02x%02x\n",
                        kd->kit.mainColor.r,
                        kd->kit.mainColor.g,
                        kd->kit.mainColor.b,
                        kd->kit.mainColor.a
                        ));
            break;

        case ATT_SECOND_COLOR:
            if (ParseColor((wchar_t*)pValue, &kd->kit.secondColor))
                kd->kit.attDefined |= SECOND_COLOR;
            GDB_DEBUG(wlog,(slog,L"secondColor = %02x%02x%02x%02x\n",
                        kd->kit.secondColor.r,
                        kd->kit.secondColor.g,
                        kd->kit.secondColor.b,
                        kd->kit.secondColor.a
                        ));
            break;

        case ATT_SOCKS_COLOR:
            if (ParseColor((wchar_t*)pValue, &kd->kit.socksColor))
                kd->kit.attDefined |= SOCKS_COLOR;
            GDB_DEBUG(wlog,(slog,L"socksColor = %02x%02x%02x%02x\n",
                        kd->kit.socksColor.r,
                        kd->kit.socksColor.g,
                        kd->kit.socksColor.b,
                        kd->kit.socksColor.a
                        ));
            break;

        case ATT_SHORTS_COLOR:
            if (ParseColor((wchar_t*)pValue, &kd->kit.shortsFirstColor))
                kd->kit.attDefined |= SHORTS_MAIN_COLOR;
            GDB_DEBUG(wlog,(slog,L"shortsFirstColor = %02x%02x%02x%02x\n",
                        kd->kit.shortsFirstColor.r,
                        kd->kit.shortsFirstColor.g,
                        kd->kit.shortsFirstColor.b,
                        kd->kit.shortsFirstColor.a
                        ));
            break;

        case ATT_ICON_TYPE:
            kd->kit.iconType = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"iconType = {%s}\n",
                        kd->kit.iconType));
            kd->kit.attDefined |= ICON_TYPE;
            break;

        case ATT_DESCRIPTION:
            kd->kit.description = (wchar_t*)pValue;
            GDB_DEBUG(wlog,(slog,L"description = {%s}\n",kd->kit.description));
            kd->kit.attDefined |= KITDESCRIPTION;
            break;

        case ATT_FRONT_NUMBER_Y:
            kd->kit.frontNumberY = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"front.number.y = {%s}\n",
                        kd->kit.frontNumberY));
            kd->kit.attDefined |= FRONT_NUMBER_Y;
            break;

        case ATT_FRONT_NUMBER_X:
            kd->kit.frontNumberX = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"front.number.x = {%s}\n",
                        kd->kit.frontNumberY));
            kd->kit.attDefined |= FRONT_NUMBER_X;
            break;

        case ATT_FRONT_NUMBER_SIZE:
            kd->kit.frontNumberSize = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"front.number.size = {%s}\n",
                        kd->kit.frontNumberSize));
            kd->kit.attDefined |= FRONT_NUMBER_SIZE;
            break;

        case ATT_SLEEVE_PATCH_LEFT_POS_SHORT:
            kd->kit.sleevePatchLeftPosShort = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"sleeve.patch.left.pos.short = {%s}\n",
                        kd->kit.sleevePatchLeftPosShort));
            kd->kit.attDefined |= SLEEVE_PATCH_LEFT_POS_SHORT;
            break;

        case ATT_SLEEVE_PATCH_LEFT_POS_LONG:
            kd->kit.sleevePatchLeftPosLong = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"sleeve.patch.left.pos.long = {%s}\n",
                        kd->kit.sleevePatchLeftPosLong));
            kd->kit.attDefined |= SLEEVE_PATCH_LEFT_POS_LONG;
            break;

        case ATT_SLEEVE_PATCH_RIGHT_POS_SHORT:
            kd->kit.sleevePatchRightPosShort = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"sleeve.patch.right.pos.short = {%s}\n",
                        kd->kit.sleevePatchRightPosShort));
            kd->kit.attDefined |= SLEEVE_PATCH_RIGHT_POS_SHORT;
            break;

        case ATT_SLEEVE_PATCH_RIGHT_POS_LONG:
            kd->kit.sleevePatchRightPosLong = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"sleeve.patch.right.pos.long = {%s}\n",
                        kd->kit.sleevePatchRightPosLong));
            kd->kit.attDefined |= SLEEVE_PATCH_RIGHT_POS_LONG;
            break;

        case ATT_SHORTS_NUMBER_SIZE:
            kd->kit.shortsNumberSize = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"shorts.number.size = {%s}\n",
                        kd->kit.shortsNumberSize));
            kd->kit.attDefined |= SHORTS_NUMBER_SIZE;
            break;

        case ATT_SHORTS_NUMBER_Y:
            kd->kit.shortsNumberY = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"shorts.number.y = {%s}\n",
                        kd->kit.shortsNumberY));
            kd->kit.attDefined |= SHORTS_NUMBER_Y;
            break;

        case ATT_SHORTS_NUMBER_X:
            kd->kit.shortsNumberX = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"shorts.number.x = {%s}\n",
                        kd->kit.shortsNumberX));
            kd->kit.attDefined |= SHORTS_NUMBER_X;
            break;

        case ATT_NUMBER_Y:
            kd->kit.numberY = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"number.y = {%s}\n",
                        kd->kit.numberY));
            kd->kit.attDefined |= NUMBER_Y;
            break;

        case ATT_NUMBER_SIZE:
            kd->kit.numberSize = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"number.size = {%s}\n",
                        kd->kit.numberSize));
            kd->kit.attDefined |= NUMBER_SIZE;
            break;

        case ATT_NAME_Y:
            kd->kit.nameY = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"name.y = {%s}\n",
                        kd->kit.nameY));
            kd->kit.attDefined |= NAME_Y;
            break;

        case ATT_NAME_SIZE:
            kd->kit.nameSize = *(DWORD*)pValue;
            GDB_DEBUG(wlog,(slog,L"name.size = {%s}\n",
                        kd->kit.nameSize));
            kd->kit.attDefined |= NAME_SIZE;
            break;
    }
}

/**
 * parses a RRGGBB(AA) string into RGBAColor structure
 */
bool ParseColor(const wchar_t* str, RGBAColor* color)
{
	int len = lstrlen(str);
	if (!(len == 6 || len == 8)) 
		return false;

	int num = 0;
	if (swscanf(str,L"%x",&num)!=1) return false;

	if (len == 6) {
		// assume alpha as fully opaque.
		color->r = (BYTE)((num >> 16) & 0xff);
		color->g = (BYTE)((num >> 8) & 0xff);
		color->b = (BYTE)(num & 0xff);
		color->a = 0xff; // set alpha to opaque
	}
	else {
		color->r = (BYTE)((num >> 24) & 0xff);
		color->g = (BYTE)((num >> 16) & 0xff);
		color->b = (BYTE)((num >> 8) & 0xff);
		color->a = (BYTE)(num & 0xff);
	}

	GDB_DEBUG(wlog, (slog, L"RGBA color: %02x,%02x,%02x,%02x\n",
				color->r, color->g, color->b, color->a));
	return true;
}

