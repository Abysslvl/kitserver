/* AFS2FS module */
#define UNICODE
#define THISMOD &k_afs

#include <windows.h>
#include <map>
#include <hash_map>
#include <string>
#include <shlobj.h>
#include <shlwapi.h>

using namespace std;
#if _CPPLIB_VER >= 503
using namespace stdext;
#endif

#include <stdio.h>
#include <sys/stat.h>
#include "kload_exp.h"
#include "afsreader.h"
#include "afsio.h"
#include "afs2fs.h"
#include "afs2fs_addr.h"
#include "dllinit.h"
//#include "gdb.h"
#include "pngdib.h"
#include "utf8.h"
#include "names.h"

//#define lang(s) getTransl("afs2fs",s)

#include <wchar.h>

#define SWAPBYTES(dw) \
    ((dw<<24 & 0xff000000) | (dw<<8  & 0x00ff0000) | \
    (dw>>8  & 0x0000ff00) | (dw>>24 & 0x000000ff))

//#define CREATE_FLAGS FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING
#define CREATE_FLAGS 0

#define MAX_AFSFILE_LEN 512

// VARIABLES
HINSTANCE hInst = NULL;
KMOD k_afs = {MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG};

// cache
#define DEFAULT_FILENAMELEN 64
#define MAX_ITEMS 13384
#define MAX_FOLDERS 32
#define MAX_BALL_NAME_LEN 64

#define EXPANSION_IMG 0x0f

typedef struct _INFO_CACHE_ENTRY_STRUCT
{
    const wchar_t* rootDir;
    wchar_t fileName[1];
} INFO_CACHE_ENTRY_STRUCT;

typedef struct _FAST_INFO_CACHE_STRUCT
{
    bool initialized; 
    int numEntries;
    size_t entrySize;
    BYTE* entries;
} FAST_INFO_CACHE_STRUCT;

class config_t 
{
public:
    config_t() : _fileNameLen(DEFAULT_FILENAMELEN) {}
    list<wstring> _roots;
    int _fileNameLen;
};

hash_map<wstring,int> g_maxItems;
hash_map<string,BYTE*> _info_cache;
FAST_INFO_CACHE_STRUCT _fast_info_cache[MAX_FOLDERS];

#define MAX_IMGDIR_LEN 4096
config_t _config;

song_map_t* _songs = NULL;
ball_map_t* _balls = NULL;

typedef struct _BALL_INFO
{
    BYTE name[MAX_BALL_NAME_LEN];
    WORD unknown1;
    WORD unknown2;
    DWORD unknown3;
} BALL_INFO;


// FUNCTIONS
HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);

void afsConfig(char* pName, const void* pValue, DWORD a);
void afsConfigImgDir(char* pName, const wchar_t* pValue, DWORD a);
void afsReadSongsMap();
bool afsGetFileInfo(DWORD afsId, DWORD binId, HANDLE& hfile, DWORD& fsize);
void afsReadBallsCallPoint();
KEXPORT DWORD afsReadBalls(BALL_INFO* balls);

// FUNCTION POINTERS

int GetNumItems(wstring& folder)
{
    static bool doOnce = true;
    int result = MAX_ITEMS;
    hash_map<wstring,int>::iterator it = g_maxItems.find(folder);
    if (it == g_maxItems.end())
    {
        // get number of files inside the corresponding AFS file
        wchar_t pesDir[MAX_PATH];
        ZeroMemory(pesDir,sizeof(pesDir));
        if (dta[PES_DIR])
        {
            char* pesDirUtf8 = *(char**)dta[PES_DIR];
            if (pesDirUtf8)
                Utf8::fUtf8ToUnicode(pesDir, pesDirUtf8);
            if (doOnce)
            {
                LOG(L"pesDir = %s", pesDir);
                doOnce = false;
            }
        }
        // get short name
        int pos = folder.rfind(L'\\');
        wstring shortName((pos != string::npos) ? 
            folder.substr(pos+1) : folder);
        
        // first try All_Users/Application_Data
        wchar_t szPath[MAX_PATH];
        if (SUCCEEDED(
                SHGetFolderPath(
                        NULL, CSIDL_COMMON_APPDATA, 
                        NULL, 0, szPath))) 
        {
            PathAppend(szPath, L"KONAMI");
            PathAppend(szPath, L"Pro Evolution Soccer 2013");
            PathAppend(szPath, L"download");

        }
        wstring afsFile(szPath);
        LOG(L"trying: {%s}", (afsFile + L'\\' + shortName).c_str());
        FILE* f = _wfopen((afsFile + L'\\' + shortName).c_str(), L"rb");
        if (!f) {
            // now try regular location
            afsFile = pesDir;
            f = _wfopen((afsFile + folder).c_str(),L"rb");
        }
        if (f) {
            AFSDIRHEADER afsDirHdr;
            ZeroMemory(&afsDirHdr,sizeof(AFSDIRHEADER));
            fread(&afsDirHdr,sizeof(AFSDIRHEADER),1,f);
            if (afsDirHdr.dwSig == AFSSIG)
            {
                g_maxItems.insert(pair<wstring,int>(folder, afsDirHdr.dwNumFiles));
                result = afsDirHdr.dwNumFiles;
            }
            fclose(f);
        }
        else
        {
            // can't open for reading, then just reserve a big enough cache
            g_maxItems.insert(pair<wstring,int>(folder, MAX_ITEMS));
        }
    }
    else
        result = it->second;

    return result;
}

void GetRelativePath(char* relativePath, char* path, size_t maxLen)
{
    if (path[0]!='\0' && path[0]!='.' && path[1]==':')
    {
        // looks like an absolute path
        char* lastSlash = strrchr(path,'\\');
        if (lastSlash)
        {
            strcpy(relativePath, ".\\img");
            strncat(relativePath, lastSlash, maxLen);
            return;
        }
    }
    strncpy(relativePath, path, maxLen);
}

bool GetBinFileName(DWORD afsId, DWORD binId, wchar_t* filename, int maxLen)
{
    char relPath[128];
    if (afsId < 0 || MAX_FOLDERS-1 < afsId) return false; // safety check
    if (!_fast_info_cache[afsId].initialized)
    {
        AFS_INFO* pBST = ((AFS_INFO**)dta[BIN_SIZES_TABLE])[afsId];
        if (pBST) 
        {
            hash_map<string,BYTE*>::iterator it;
            for (it = _info_cache.begin(); it != _info_cache.end(); it++)
            {
                ZeroMemory(relPath,sizeof(relPath));
                GetRelativePath(relPath, pBST->filename,
                        sizeof(relPath));
                if (stricmp(it->first.c_str(), relPath)==0)
                    _fast_info_cache[afsId].entries = it->second;
            }
        }
        if (k_afs.debug)
            LOG(L"initialized _fast_info_cache entry for afsId=0x%02x", afsId);

        _fast_info_cache[afsId].numEntries = pBST->numItems;
        _fast_info_cache[afsId].entrySize = sizeof(wchar_t*) + sizeof(wchar_t)*_config._fileNameLen;
        _fast_info_cache[afsId].initialized = true;
    }

    TRACE(L"Loading afsId=0x%02x, binId=%d", afsId, binId);

    BYTE* base = _fast_info_cache[afsId].entries;
    if (binId >= _fast_info_cache[afsId].numEntries || !base)
        return false;
    INFO_CACHE_ENTRY_STRUCT* entry = (INFO_CACHE_ENTRY_STRUCT*)(
                _fast_info_cache[afsId].entries + binId*(sizeof(wchar_t*) + sizeof(wchar_t)*_config._fileNameLen));
    if (entry->fileName[0]==L'\0')
        return false;

    AFS_INFO* pBST = ((AFS_INFO**)dta[BIN_SIZES_TABLE])[afsId];
    wchar_t afsDir[MAX_AFSFILE_LEN];
    ZeroMemory(afsDir,sizeof(afsDir));
    ZeroMemory(relPath,sizeof(relPath));
    GetRelativePath(relPath, pBST->filename, sizeof(relPath));
    Utf8::fUtf8ToUnicode(afsDir, (BYTE*)relPath);
    _snwprintf(filename, maxLen, L"%s\\%s\\%s", 
            entry->rootDir, afsDir, entry->fileName);
    return true;
}

void InitializeFileNameCache()
{
    LOG(L"Initializing filename cache...");
    for (list<wstring>::iterator lit = _config._roots.begin();
            lit != _config._roots.end();
            lit++)
    {
        WIN32_FIND_DATA fData;
        wstring pattern(lit->c_str());
        pattern += L"\\img\\*.img";
        LOG(L"processing afs root {%s}",lit->c_str());

        HANDLE hff = FindFirstFile(pattern.c_str(), &fData);
        if (hff == INVALID_HANDLE_VALUE) 
        {
            // none found.
            continue;
        }
        while(true)
        {
            // check if this is a directory
            if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (wcscmp(fData.cFileName,L".")==0 ||
                    wcscmp(fData.cFileName,L"..")==0)
                    continue;

                WIN32_FIND_DATA fData1;
                wstring folder(L".\\img\\");
                folder += fData.cFileName;
                wstring folderpattern(*lit);
                folderpattern += L"\\";
                folderpattern += folder + L"\\*";

                char* key_cstr = (char*)Utf8::unicodeToUtf8(folder.c_str());
                string key(key_cstr);
                Utf8::free(key_cstr);
                TRACE(L"folderpattern = {%s}", folderpattern.c_str());

                HANDLE hff1 = FindFirstFile(folderpattern.c_str(), &fData1);
                while (hff1 != INVALID_HANDLE_VALUE)
                {
                    // check if this is a file
                    if (!(fData1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
                    {
                        int binId = -1;
                        wchar_t* s = wcsrchr(fData1.cFileName,'_');
                        if (s && swscanf(s+1,L"%d",&binId)==1)
                        {
                            TRACE(L"folder={%s}, bin={%d}",
                                    folder.c_str(),binId);
                            if (binId >= 0)
                            {
                                BYTE* entries = NULL;
                                hash_map<string,BYTE*>::iterator cit;
                                cit = _info_cache.find(key);
                                if (cit != _info_cache.end()) 
                                    entries = cit->second;
                                else 
                                {
                                    entries = (BYTE*)HeapAlloc(
                                        GetProcessHeap(),
                                        HEAP_ZERO_MEMORY, 
                                        (sizeof(wchar_t*) 
                                            + sizeof(wchar_t) *
                                            _config._fileNameLen) *
                                            GetNumItems(folder)
                                        );
                                    _info_cache.insert(pair<string,BYTE*>(
                                        key, entries));
                                }

                                if (binId >= GetNumItems(folder))
                                {
                                    // binID too large
                                    LOG(L"ERROR: bin ID for filename \"%s\" is too large. Maximum bin ID for this folder is: %d", 
                                        fData1.cFileName, 
                                        GetNumItems(folder)-1);
                                }
                                else if (wcslen(fData1.cFileName) >= 
                                        _config._fileNameLen)
                                {
                                    // file name too long
                                    LOG(L"ERROR: filename too long: \"%s\" (in folder: %s)", 
                                        fData1.cFileName, folder.c_str());
                                    LOG(L"ERROR: length = %d chars. Maximum allowed length: %d chars.", 
                                        wcslen(fData1.cFileName), 
                                        _config._fileNameLen-1);
                                }
                                else
                                {
                                    INFO_CACHE_ENTRY_STRUCT* entry = 
                                        (INFO_CACHE_ENTRY_STRUCT*)(entries + 
                                        binId*(sizeof(wchar_t*) + 
                                        sizeof(wchar_t)*_config._fileNameLen));
                                    // put filename into cache
                                    wcsncpy(
                                        entry->fileName, 
                                        fData1.cFileName, 
                                        _config._fileNameLen-1);
                                    // put foldername into cache
                                    entry->rootDir = lit->c_str();
                                }
                            }
                        }
                    }

                    // proceed to next file
                    if (!FindNextFile(hff1, &fData1)) break;
                }
                FindClose(hff1);
            }

            // proceed to next file
            if (!FindNextFile(hff, &fData)) break;
        }
        FindClose(hff);

    } // for roots

    // print cache
    for (hash_map<wstring,int>::iterator it = g_maxItems.begin(); 
            it != g_maxItems.end(); 
            it++)
        LOG(L"filename cache: {%s} : %d slots", it->first.c_str(), it->second);

    LOG(L"DONE initializing filename cache.");
}

/*******************/
/* DLL Entry Point */
/*******************/
EXTERN_C BOOL WINAPI DllMain(
        HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;

		RegisterKModule(THISMOD);
		
		if (!checkGameVersion()) {
			LOG(L"Sorry, your game version isn't supported!");
			return false;
		}

        CHECK_KLOAD(MAKELONG(0, KITSERVER_VERSION_MAJOR));
        
		copyAdresses();
		hookFunction(hk_D3D_CreateDevice, initModule);
	}
	
	else if (dwReason == DLL_PROCESS_DETACH)
	{
        LOG(L"freeing up info_cache memory...");
        for (hash_map<string,BYTE*>::iterator it = _info_cache.begin(); 
                it != _info_cache.end();
                it++)
            if (it->second) HeapFree(GetProcessHeap(), 0, it->second);

        if (_songs) delete _songs;
        if (_balls) delete _balls;
        LOG(L"done");
	}
	
	return true;
}

void afsConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1: // debug
			k_afs.debug = *(DWORD*)pValue;
			break;
		case 2: // filename.length
			_config._fileNameLen = *(DWORD*)pValue;
			break;
	}
	return;
}

void afsConfigImgDir(char* pName, const wchar_t* pValue, DWORD a)
{	
    wchar_t* s = (wchar_t*)pValue;
    if (s[1]==L':')
    {
        // absolute root
        wstring root(s);
        _config._roots.push_back(root);
        LOG(L"Config: afs.root: {%s}", root.c_str());
    }
    else
    {
        // relative root
        wstring root(getPesInfo()->myDir);
        root += L"afs-root\\";
        root += s;
        _config._roots.push_back(root);
        LOG(L"Config: afs.root: {%s}", root.c_str());
    }
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface) {

    getConfig("afs2fs", "debug", DT_DWORD, 1, afsConfig);
    getConfig("afs2fs", "filename.length", DT_DWORD, 2, afsConfig);
    getConfig("afs2fs", "afs.root", DT_STRING, C_ALL, 
            (PROCESSCONFIG)afsConfigImgDir);

	unhookFunction(hk_D3D_CreateDevice, initModule);

    // register callback
    afsioAddCallback(afsGetFileInfo);

    InitializeFileNameCache();
    ZeroMemory(_fast_info_cache,sizeof(_fast_info_cache));

    HookCallPoint(code[C_READ_BALLS], afsReadBallsCallPoint, 6, 2);
	TRACE(L"Hooking done.");

    if (getPesInfo()->gameVersion >= gvPES2013demo)
    {
        BYTE* bptr = (BYTE*)dta[SONGS_INFO_TABLE];
        if (bptr)
        {
            _songs = new song_map_t();
            for (list<wstring>::iterator lit = _config._roots.begin();
                    lit != _config._roots.end();
                    lit++)
            {
                wstring songMapFile(lit->c_str());
                songMapFile += L"\\songs.txt";
                _songs->update(songMapFile);
            }

            // apply songs info
            DWORD protection = 0;
            DWORD newProtection = PAGE_READWRITE;
            int numSongs = dta[NUM_SONGS];
            if (VirtualProtect(
                    bptr, numSongs*sizeof(SONG_STRUCT), 
                    newProtection, &protection)) 
            {
                SONG_STRUCT* ss = (SONG_STRUCT*)dta[SONGS_INFO_TABLE];
                for (int i=0; i<numSongs; i++)
                {
                    hash_map<WORD,SONG_STRUCT>::iterator it;
                    it = _songs->_songMap.find(ss[i].binId);
                    if (it != _songs->_songMap.end())
                    {
                        ss[i].title = it->second.title;
                        ss[i].author = it->second.author;
                        LOG(L"Set title/artist info for song with binId=%d",
                                ss[i].binId);
                    }
                }
            }
        }

        _balls = new ball_map_t();
        for (list<wstring>::iterator lit = _config._roots.begin();
                lit != _config._roots.end();
                lit++)
        {
            wstring ballMapFile(lit->c_str());
            ballMapFile += L"\\balls.txt";
            _balls->update(ballMapFile);
        }
    }

    //__asm { int 3 }          // uncomment this for debugging as needed
    return D3D_OK;
}

/**
 * Simple file-check routine.
 */
bool OpenFileIfExists(const wchar_t* filename, HANDLE& handle, DWORD& size)
{
    TRACE(L"OpenFileIfExists:: %s", filename);
    handle = CreateFile(filename,           // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | CREATE_FLAGS, // normal file
                       NULL);                 // no attr. template

    if (handle != INVALID_HANDLE_VALUE)
    {
        size = GetFileSize(handle,NULL);
        TRACE(L"OpenFileIfExists: handle = %08x", (DWORD)handle);
        return true;
    }
    return false;
}

/**
 * AFSIO callback
 */
bool afsGetFileInfo(DWORD afsId, DWORD binId, HANDLE& hfile, DWORD& fsize)
{
    wchar_t filename[MAX_AFSFILE_LEN];
    if (!GetBinFileName(afsId, binId, filename, 
                sizeof(filename)/sizeof(wchar_t)))
        return false; // quick return

    TRACE(L"file = {%s}",filename);
    return OpenFileIfExists(filename, hfile, fsize);
}

void afsReadBallsCallPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push eax // parameter: balls-base
        call afsReadBalls
        add esp,4  // pop params
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov byte ptr ds:[esi+0x3c],1
        mov esi,dword ptr ds:[esi+0x34]
        retn
    }
}

KEXPORT DWORD afsReadBalls(BALL_INFO* balls)
{
    // apply balls info
    BYTE* bptr = (BYTE*)balls;
    DWORD protection = 0;
    DWORD newProtection = PAGE_READWRITE;
    if (VirtualProtect(bptr, dta[NUM_BALLS]*sizeof(BALL_INFO), 
                newProtection, &protection)) 
    {
        for (int i=0; i<dta[NUM_BALLS]; i++)
        {
            hash_map<WORD,BALL_STRUCT>::iterator it = 
                    _balls->_ballMap.find(i+1);
            if (it != _balls->_ballMap.end())
            {
                memset(balls[i].name, 0, MAX_BALL_NAME_LEN);
                strncpy((char*)balls[i].name, 
                        it->second.name, MAX_BALL_NAME_LEN-1);
            }
        }
    }

    // print current balls info
    for (int i=0; i<dta[NUM_BALLS]; i++)
    {
        wchar_t* ballName = Utf8::utf8ToUnicode(balls[i].name);
        LOG(L"ball #%d: {%s}", i+1, ballName);
        Utf8::free(ballName);
    }

    // restore code
    //PatchCode(code[C_READ_BALLS], "\xc6\x46\x3c\x01\x8b\x76\x34", 7); 
    return 0;
}
