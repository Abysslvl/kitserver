// hook.cpp
#define UNICODE
#define THISMOD &k_kload

#include <windows.h>
#include <stdio.h>
#include "shared.h"
#include "log.h"
#include "manage.h"
#include "hook.h"
#include "kload.h"
#include "hook_addr.h"
#include "utf8.h"
#include "crc32.h"
#define lang(s) getTransl("kload",s)

#include <vector>
#include <algorithm>
#include <map>
#include <hash_map>
#include <string>

//#include "apihijack.h"
//#include "d3d9.h"


extern KMOD k_kload;
extern PESINFO g_pesinfo;
extern HINSTANCE hInst;
IDirect3DDevice9* g_device = NULL;
vector<void*> g_callChains[hk_LAST];
//ID3DXFont* myFonts[4][100];
bool g_bGotFormat = false;
bool rpPrepared = false;
bool freeEditModeData = false;
extern bool _noshade;


map<DWORD*, DWORD> g_replacedHeaders;
map<DWORD*, DWORD>::iterator g_replacedHeadersIt;
hash_map<DWORD, IDirect3DTexture9*> g_replacedMap;
hash_map<DWORD, IDirect3DTexture9*>::iterator g_replacedMapIt;
	
hash_map<DWORD, EditPlayerInfo*> g_editPlayerInfoMap;
hash_map<DWORD, EditPlayerInfo*>::iterator g_editPlayerInfoMapIt;


PFNDIRECT3DCREATE9PROC g_orgDirect3DCreate9 = NULL;
BYTE g_codeDirect3DCreate9[5] = {0,0,0,0,0};
PFNCREATEDEVICEPROC g_orgCreateDevice = NULL;
PFNPRESENTPROC g_orgPresent = NULL;
PFNRESETPROC g_orgReset = NULL;
PFNSETTRANSFORMPROC g_orgSetTransform = NULL;

#define MAX_OVERLAYS 40
bool g_overlayOn = false;
int g_overlayPage = -1;
list<OVERLAY_EVENT_CALLBACK> _overlay_callbacks;
bool _activeOverlayPages[MAX_OVERLAYS];
int _numOverlayPages = 0;

DWORD g_menuMode = 0;
list<MENU_EVENT_CALLBACK> _menu_callbacks;

void hookAddMenuModeCallPoint();
void hookSubMenuModeCallPoint();
void hookTriggerSelectionOverlay(int delta);

void hookPrepareDataToWriteCallPoint();
void hookPrepareDataToWrite(LPVOID buffer, DWORD numBytes);
void hookAfterReadDataCallPoint();
void hookAfterReadData(LPVOID buffer, DWORD numBytes);

HHOOK g_hKeyboardHook = NULL;
list<KEY_EVENT_CALLBACK> _key_callbacks;
void HookKeyboard();
void UnhookKeyboard();
LRESULT CALLBACK KeyboardProc(int code1, WPARAM wParam, LPARAM lParam);
HRESULT InvalidateDeviceObjects(IDirect3DDevice9* device);
HRESULT RestoreDeviceObjects(IDirect3DDevice9* device);

typedef BOOL (WINAPI *WRITEFILE_PROC)(
  HANDLE hFile,
  LPCVOID lpBuffer,
  DWORD nNumberOfBytesToWrite,
  LPDWORD lpNumberOfBytesWritten,
  LPOVERLAPPED lpOverlapped
);
typedef BOOL (WINAPI *READFILE_PROC)(
  HANDLE hFile,
  LPCVOID lpBuffer,
  DWORD nNumberOfBytesToRead,
  LPDWORD lpNumberOfBytesRead,
  LPOVERLAPPED lpOverlapped
);
WRITEFILE_PROC _writeFile = NULL;
READFILE_PROC _readFile = NULL;

BOOL WINAPI hookWriteFile(
  HANDLE hFile,
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToWrite,
  LPDWORD lpNumberOfBytesWritten,
  LPOVERLAPPED lpOverlapped
);
BOOL WINAPI hookReadFile(
  HANDLE hFile,
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToRead,
  LPDWORD lpNumberOfBytesRead,
  LPOVERLAPPED lpOverlapped
);

typedef DWORD (*COPYDATA_PROC)(DWORD dest, DWORD src, DWORD len);
DWORD _org_copy;

void NextPage();

void enterUniformSelectCallPoint();
void exitUniformSelectCallPoint();
KEXPORT void enterUniformSelect();
KEXPORT void exitUniformSelect();
list<UNIFORM_SELECT_EVENT_CALLBACK> _uniformSelectEventCallbacks;

void hookAtCopyEditDataCallPoint1();
void hookAtCopyEditDataCallPoint2();
void hookCopyPlayerData(
    PLAYER_INFO* players, DWORD numBytes, int place, bool writeList=true);
void hookAtCopyEditData(DWORD dest, DWORD numBytes, int place);
list<COPY_PLAYER_DATA_CALLBACK> _copyPlayerDataCallbacks;

void hookAfterReadNamesCallPoint();
void hookAfterReadNamesCallPoint2();
void hookAfterReadNames();
list<READ_NAMES_CALLBACK> _readNamesCallbacks;

list<READ_DATA_CALLBACK> _writeEditDataCallbacks;
list<READ_DATA_CALLBACK> _writeReplayDataCallbacks;
list<READ_DATA_CALLBACK> _writeBalDataCallbacks;
list<READ_DATA_CALLBACK> _readEditDataCallbacks;
list<READ_DATA_CALLBACK> _readReplayDataCallbacks;
list<READ_DATA_CALLBACK> _readBalDataCallbacks;

ALLVOID g_orgBeginRender1 = NULL;
ALLVOID g_orgBeginRender2 = NULL;
DWORD g_orgEditCopyPlayerName = NULL;

string renderedPlayers = "";

///// Graphics //////////////////

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_CUSTOMVERTEX2 (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct CUSTOMVERTEX { 
	FLOAT x,y,z,w;
	DWORD color;
};

struct CUSTOMVERTEX2 { 
	FLOAT x,y,z,w;
	DWORD color;
	FLOAT tu, tv;
};

CUSTOMVERTEX g_shaded[] = {
	{0.0f, 0.0f, 0.0f, 1.0f, 0xcc000000}, //1
	{0.0f, 66.0f, 0.0f, 1.0f, 0x00707070}, //2
	{1024.0f, 0.0f, 0.0f, 1.0f, 0xcc000000}, //3
	{1024.0f, 66.0f, 0.0f, 1.0f, 0x00707070}, //4
};

// shaded zone
static IDirect3DVertexBuffer9* g_pVB_shaded = NULL;

static IDirect3DStateBlock9* g_sb_them = NULL;
static IDirect3DStateBlock9* g_sb_me = NULL;

void SetPosition(CUSTOMVERTEX* dest, CUSTOMVERTEX* src, int n, int x, int y) 
{
    FLOAT xratio = getPesInfo()->bbWidth / 1024.0;
    FLOAT yratio = getPesInfo()->bbHeight / 768.0;
    for (int i=0; i<n; i++) {
        dest[i].x = (FLOAT)(int)((src[i].x + x) * xratio);
        dest[i].y = (FLOAT)(int)((src[i].y + y) * yratio);
    }
}

void hookDirect3DCreate9()
{
	BYTE g_jmp[5] = {0,0,0,0,0};
	// Put a JMP-hook on Direct3DCreate9
	TRACE(L"JMP-hooking Direct3DCreate9...");
	
	char d3dDLL[5];
	switch (g_pesinfo.gameVersion) {
	default:
		strcpy(d3dDLL, "d3d9");
		break;
	};
	
	HMODULE hD3D9 = GetModuleHandleA(d3dDLL);
	if (!hD3D9) {
	    hD3D9 = LoadLibraryA(d3dDLL);
	}
	if (hD3D9) {
		g_orgDirect3DCreate9 = (PFNDIRECT3DCREATE9PROC)GetProcAddress(hD3D9, "Direct3DCreate9");
		
		// unconditional JMP to relative address is 5 bytes.
		g_jmp[0] = 0xe9;
		DWORD addr = (DWORD)newDirect3DCreate9 - (DWORD)g_orgDirect3DCreate9 - 5;
		TRACE(L"New address for Direct3DCreate9: JMP %08x", addr);
		memcpy(g_jmp + 1, &addr, sizeof(DWORD));
		
		memcpy(g_codeDirect3DCreate9, g_orgDirect3DCreate9, 5);
		DWORD newProtection = PAGE_EXECUTE_READWRITE;
		DWORD g_savedProtection;
		if (VirtualProtect(g_orgDirect3DCreate9, 8, newProtection, &g_savedProtection))
		{
		    memcpy(g_orgDirect3DCreate9, g_jmp, 5);
		    TRACE(L"JMP-hook planted.");
		}
	}
	return;
}

KEXPORT void hookFunction(HOOKS h, void* addr)
{
	if (h >= hk_LAST) return;
    //LOG(L"hookFunction: %d", (DWORD)h);
	
	g_callChains[h].push_back(addr);
}

KEXPORT void unhookFunction(HOOKS h, void* addr)
{
	if (h >= hk_LAST) return;
    //LOG(L"unhookFunction: %d", (DWORD)h);
		
    // we need "erase", because "remove" doesn't actually remove the items, but
    // instead re-arranges them.
    EnterCriticalSection(&g_cs);
    g_callChains[h].erase(
            remove(g_callChains[h].begin(),	g_callChains[h].end(), addr),
            g_callChains[h].end());
    LeaveCriticalSection(&g_cs);
}

void initAddresses()
{
	// select correct addresses
	memcpy(code, codeArray[g_pesinfo.gameVersion], sizeof(code));
	memcpy(dta, dataArray[g_pesinfo.gameVersion], sizeof(dta));
	memcpy(ltfpPatch, ltfpPatchArray[g_pesinfo.gameVersion], sizeof(ltfpPatch));
	
	//ZeroMemory(myFonts, 4*100*sizeof(ID3DXFont*));
	
	return;
}

int callCount = 0;
IDirect3D9* STDMETHODCALLTYPE newDirect3DCreate9(UINT sdkVersion) {

    // put back saved code fragment
    BYTE* dest = (BYTE*)g_orgDirect3DCreate9;
    dest[0] = g_codeDirect3DCreate9[0];
    *((DWORD*)(dest + 1)) = *((DWORD*)(g_codeDirect3DCreate9 + 1));

	// the first instance is freed after getting the device properties
    callCount += 1;
    LOG(L"callCount = %d", callCount);

    if (callCount == 1) {
		// process the calls to the modules
		ALLVOID NextCall = NULL;
	
		LOG(L"newDirect3DCreate9 called.");
		
        CALLCHAIN_BEGIN(hk_D3D_Create, it) 
        {
			NextCall = (ALLVOID)*it;
			NextCall();
        } 
        CALLCHAIN_END
	}

	// call the original function.
	IDirect3D9* result = g_orgDirect3DCreate9(sdkVersion);
	
	if (!g_device) {
		DWORD* vtable = (DWORD*)(*((DWORD*)result));
        
	    // hook CreateDevice method
        PFNCREATEDEVICEPROC createDeviceProc =
            (PFNCREATEDEVICEPROC)vtable[VTAB_CREATEDEVICE];
        if (createDeviceProc != newCreateDevice) {
            g_orgCreateDevice = createDeviceProc;
        }
	    DWORD protection = 0;
		DWORD newProtection = PAGE_EXECUTE_READWRITE;
		if (VirtualProtect(
                vtable+VTAB_CREATEDEVICE, 4, newProtection, &protection))
		{
			vtable[VTAB_CREATEDEVICE] = (DWORD)newCreateDevice;
			TRACE(L"CreateDevice hooked.");
		}
        else {
            LOG(L"Unable to hook CreateDevice.");
        }
	}
    else {
        LOG(L"g_device = %p", g_device);
    }
	
	//hookOthers();


    //TODO: make this only on strip selection
    //HookKeyboard();
	
    hookDirect3DCreate9(); //rehook
	return result;
}

void hookOthers() {

	DWORD newProtection = PAGE_EXECUTE_READWRITE;
	DWORD g_savedProtection;
	BYTE* bptr;
	DWORD* ptr;

	if (code[C_LOADTEXTUREFORPLAYER] != 0 && code[C_LOADTEXTUREFORPLAYER_CS] != 0)	{
		bptr = (BYTE*)code[C_LOADTEXTUREFORPLAYER_CS];
    if (VirtualProtect(bptr, 5 + LTFPLEN, newProtection, &g_savedProtection)) {
        bptr[0] = 0xe8;
        DWORD* ptr = (DWORD*)(code[C_LOADTEXTUREFORPLAYER_CS] + 1);
        ptr[0] = (DWORD)hookedLoadTextureForPlayer - (DWORD)(code[C_LOADTEXTUREFORPLAYER_CS] + 5);
        
        memcpy(bptr + 5, ltfpPatch, LTFPLEN);
    }
	}
	
	// replace the render functions for player textures	
	ptr = (DWORD*)code[TBL_BEGINRENDER1];
	VirtualProtect(ptr, 4, newProtection, &g_savedProtection);
	g_orgBeginRender1 = (ALLVOID)*ptr;
	*ptr = (DWORD)hookedBeginRenderPlayer;
	
	ptr = (DWORD*)code[TBL_BEGINRENDER2];
	VirtualProtect(ptr, 4, newProtection, &g_savedProtection);
	g_orgBeginRender2 = (ALLVOID)*ptr;
	*ptr = (DWORD)hookedBeginRenderPlayer;
	
	if (code[C_EDITCOPYPLAYERNAME_CS] != 0)	{
		bptr = (BYTE*)code[C_EDITCOPYPLAYERNAME_CS];
    if (VirtualProtect(bptr, 5, newProtection, &g_savedProtection)) {
        bptr[0] = 0xe8;
        DWORD* ptr = (DWORD*)(code[C_EDITCOPYPLAYERNAME_CS] + 1);
        g_orgEditCopyPlayerName = *ptr + code[C_EDITCOPYPLAYERNAME_CS] + 5;
        ptr[0] = (DWORD)hookedEditCopyPlayerName - (DWORD)(code[C_EDITCOPYPLAYERNAME_CS] + 5);
    }
	}
	
	MasterHookFunction(code[C_COPYSTRING_CS], 4, hookedCopyString);
	
	return;
}

HRESULT STDMETHODCALLTYPE newCreateDevice(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
	LOG(L"newCreateDevice called.");
	
	HRESULT result = g_orgCreateDevice(self, Adapter, DeviceType, hFocusWindow,
            BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
            
	g_device = *ppReturnedDeviceInterface;
	g_device->AddRef();

	if (0) {//g_device) {
		DWORD* vtable = (DWORD*)(*((DWORD*)g_device));
		// the first vtable is overwritten by the second one on resets
		DWORD* vtable2 = *(DWORD**)(vtable - 1);
		DWORD protection = 0, protection2 = 0;
		DWORD newProtection = PAGE_EXECUTE_READWRITE;

		// hook Present method
    if (!g_orgPresent) {
			g_orgPresent = (PFNPRESENTPROC)vtable[VTAB_PRESENT];
			if (VirtualProtect(vtable+VTAB_PRESENT, 4, newProtection, &protection) &&
					VirtualProtect(vtable2+VTAB_PRESENT, 4, newProtection, &protection))
			{
				vtable[VTAB_PRESENT] = vtable2[VTAB_PRESENT] = (DWORD)newPresent;
				TRACE(L"Present hooked.");
			}
    }
    
    // hook Reset method
    if (!g_orgReset) {
			g_orgReset = (PFNRESETPROC)vtable[VTAB_RESET];
			if (VirtualProtect(vtable+VTAB_RESET, 4, newProtection, &protection) &&
					VirtualProtect(vtable2+VTAB_RESET, 4, newProtection, &protection))
			{
				vtable[VTAB_RESET] = vtable2[VTAB_RESET] = (DWORD)newReset;
				TRACE(L"Reset hooked.");
			}
    }

    /*
    // hook SetTransform method
    if (!g_orgSetTransform) {
			g_orgSetTransform = (PFNSETTRANSFORMPROC)vtable[VTAB_SETTRANSFORM];
			if (VirtualProtect(vtable+VTAB_SETTRANSFORM, 4, newProtection, &protection) &&
					VirtualProtect(vtable2+VTAB_SETTRANSFORM, 4, newProtection, &protection))
			{
				vtable[VTAB_SETTRANSFORM] = vtable2[VTAB_SETTRANSFORM] = (DWORD)newSetTransform;
				TRACE(L"SetTransform hooked.");
			}
    }
    */
	}

    g_menuMode = dta[MENU_MODE_IDX];
    HookCallPoint(code[C_ADD_MENUMODE], hookAddMenuModeCallPoint, 6, 2);
    HookCallPoint(code[C_SUB_MENUMODE], hookSubMenuModeCallPoint, 6, 2);

    //_writeFile = (WRITEFILE_PROC)HookIndirectCall(code[C_WRITE_FILE],
    //        hookWriteFile);
    //_readFile = (READFILE_PROC)HookIndirectCall(code[C_READ_FILE],
    //        hookReadFile);
    HookCallPoint(code[C_WRITE_DATA], 
        hookPrepareDataToWriteCallPoint, 6, 1);
    HookCallPoint(code[C_READ_DATA], 
        hookAfterReadDataCallPoint, 6, 0);
    
    //HookIndirectCall2(code[C_WRITE_FILE], hookWriteFile);
    //HookIndirectCall2(code[C_READ_FILE], hookReadFile);

    /**
    SDLLHook Kernel32Hook = 
    {
      "KERNEL32.DLL",
      false, NULL,		// Default hook disabled, NULL function pointer.
      {
          { "ReadFile", hookReadFile },
          { "WriteFile", hookWriteFile },
          { NULL, NULL }
      }
    };
    HookAPICalls( &Kernel32Hook );
    **/
    HookCallPoint(code[C_ENTER_UNIFORM_SELECT], 
            enterUniformSelectCallPoint, 6, 2);
    HookCallPoint(code[C_EXIT_UNIFORM_SELECT], 
            exitUniformSelectCallPoint, 6, 0);

    _org_copy = GetTargetAddress(code[C_COPY_DATA]);
    HookCallPoint(
        code[C_COPY_DATA], hookAtCopyEditDataCallPoint1, 6, 0);
    HookCallPoint(
        code[C_COPY_DATA2], hookAtCopyEditDataCallPoint2, 6, 1);

    HookCallPoint(code[C_READ_NAMES], 
            hookAfterReadNamesCallPoint, 6, 1);
    //HookCallPoint(code[C_READ_NAMES2], 
    //        hookAfterReadNamesCallPoint2, 6, 1);

	CALLCHAIN_BEGIN(hk_D3D_CreateDevice, it) {
		PFNCREATEDEVICEPROC NextCall = (PFNCREATEDEVICEPROC)*it;
		NextCall(self, Adapter, DeviceType, hFocusWindow,
           	BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	} CALLCHAIN_END

    //HMODULE hmod = GetModuleHandle(L"dfengine.dll");
    //if (hmod != NULL) {
    //    LOG(L"unloading dfengine.dll...");
    //    FreeLibrary(hmod);
    //}
    //hmod = GetModuleHandle(L"wbhelp.dll");
    //if (hmod != NULL) {
    //    LOG(L"unloading wbhelp.dll...");
    //    FreeLibrary(hmod);
    //}
	return result;
}

KEXPORT IDirect3DDevice9* getActiveDevice()
{
	return g_device;
}

KEXPORT void KDrawTextAbsolute(const wchar_t* str, UINT x, UINT y, D3DCOLOR color, float fontSize, BYTE attr, DWORD format) {
    /*
	RECT rect = {x, y, 0xffff, 0xffff};
    if (format & DT_CENTER)
    {
        rect.left = x-200;
        rect.right = x+200;
        rect.top = y-fontSize/1.5;
        rect.bottom = y+fontSize/1.5;
    }
	
	if (!myFonts[attr%4][BYTE(2*fontSize)])
		D3DXCreateFont(g_device, fontSize * g_pesinfo.stretchY, 0, (attr & KDT_BOLD)?FW_BOLD:0, 0, attr & KDT_ITALIC, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &(myFonts[attr%4][BYTE(2*fontSize)]));
	
	ID3DXFont* myFont = myFonts[attr%4][BYTE(2*fontSize)];
	
	bool needsEndScene = SUCCEEDED(g_device->BeginScene());
	
	myFont->DrawText(NULL, str, -1, &rect, format, color);
	
	if (needsEndScene)
		g_device->EndScene();
        */
	return;
}

KEXPORT void KDrawText(const wchar_t* str, UINT x, UINT y, D3DCOLOR color, float fontSize, BYTE attr, DWORD format) {
	KDrawTextAbsolute(str, x*g_pesinfo.stretchX, y*g_pesinfo.stretchY, color, fontSize, attr, format);
	return;
}

HRESULT InvalidateDeviceObjects(IDirect3DDevice9* device)
{
    LOG(L"Invalidating device objects.");
    // delete state blocks
    if (g_sb_them) { g_sb_them->Release(); g_sb_them=NULL; }
    if (g_sb_me) { g_sb_me->Release(); g_sb_me=NULL; }
    return S_OK;
}

HRESULT RestoreDeviceObjects(IDirect3DDevice9* device)
{
    // create vertex buffers
    if (!g_pVB_shaded)
    {
        BYTE* pVertices;
        if (FAILED(device->CreateVertexBuffer(sizeof(g_shaded), D3DUSAGE_WRITEONLY, 
                        D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &g_pVB_shaded, NULL)))
        {
            LOG(L"CreateVertexBuffer() failed.");
            return E_FAIL;
        }
        LOG(L"CreateVertexBuffer() done.");

        if (FAILED(g_pVB_shaded->Lock(0, sizeof(g_shaded), (void**)&pVertices, 0)))
        {
            LOG(L"g_pVB_shaded->Lock() failed.");
            return E_FAIL;
        }
        memcpy(pVertices, g_shaded, sizeof(g_shaded));
        SetPosition((CUSTOMVERTEX*)pVertices, g_shaded, sizeof(g_shaded)/sizeof(CUSTOMVERTEX), 0, 0);
        g_pVB_shaded->Unlock();
    }

	// Create the state blocks
    if (!g_sb_them || !g_sb_me)
    {
        LOG(L"Preparing state blocks...");
        DWORD numPasses;
        device->ValidateDevice(&numPasses);

        for( UINT which=0; which<2; which++ )
        {
            device->BeginStateBlock();

            device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
            device->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
            device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
            device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
            device->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
            device->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );
            device->SetRenderState( D3DRS_STENCILENABLE,    false );
            device->SetRenderState( D3DRS_CLIPPING, true );
            device->SetRenderState( D3DRS_CLIPPLANEENABLE, false );
            device->SetRenderState( D3DRS_FOGENABLE,        false );

            device->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_DISABLE );
            device->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

            device->SetVertexShader( NULL );
            device->SetFVF( D3DFVF_CUSTOMVERTEX );
            device->SetPixelShader( NULL );
            device->SetTexture(0, NULL);
            device->SetStreamSource( 0, g_pVB_shaded, 0, sizeof(CUSTOMVERTEX));

            if( which==0 )
            {
                device->EndStateBlock( &g_sb_them );
            }
            else
            {
                device->EndStateBlock( &g_sb_me );
            }
        }
    }

    LOG(L"RestoreDeviceObjects DONE.");
    return S_OK;
}

bool NeedShade()
{
    for (int i=0; i<_numOverlayPages; i++)
        if (_activeOverlayPages[i])
        {
            if (g_overlayPage == -1) 
                g_overlayPage = i;
            return true;
        }
    return false;
}

void ResetOverlayPage()
{
    g_overlayPage = -1;
}

HRESULT STDMETHODCALLTYPE newPresent(IDirect3DDevice9* self, CONST RECT* src, CONST RECT* dest,
	HWND hWnd, LPVOID unused)
{
	
	if (!g_bGotFormat) {
		kloadGetBackBufferInfo(self);
	}

    if (!g_callChains[hk_D3D_Present].empty())
    {
        if (NeedShade() && !_noshade && g_sb_them && g_sb_me && g_pVB_shaded)
        {
            g_sb_them->Capture();
            g_sb_me->Apply();

            // draw shaded zone
            self->BeginScene();
            self->SetStreamSource(0, g_pVB_shaded, 0, sizeof(CUSTOMVERTEX));
            self->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
            self->EndScene();

            g_sb_them->Apply();
        }

        // call the callbacks
        CALLCHAIN(hk_D3D_Present, it) {
            PFNPRESENTPROC NextCall=(PFNPRESENTPROC)*it;
            NextCall(self, src, dest, hWnd, unused);
        }
    }

    /*
	wchar_t* rp = Utf8::ansiToUnicode(renderedPlayers.c_str());
	KDrawText(rp, 0, 0, D3DCOLOR_RGBA(255,0,0,192));
	Utf8::free(rp);
	
	renderedPlayers = "";
	
	// free all replaced buffers and reset them to the original ones
	for (g_replacedHeadersIt = g_replacedHeaders.begin(); 
			g_replacedHeadersIt != g_replacedHeaders.end();
			g_replacedHeadersIt++)
		{
			HeapFree(GetProcessHeap(), 0, (VOID*)*(g_replacedHeadersIt->first));
			*(g_replacedHeadersIt->first) = g_replacedHeadersIt->second;
		}
	g_replacedHeaders.clear();
	
	rpPrepared = false;
    */

	// CALL ORIGINAL FUNCTION ///////////////////
	HRESULT res = g_orgPresent(self, src, dest, hWnd, unused);

	return res;
}

HRESULT STDMETHODCALLTYPE newReset(IDirect3DDevice9* self, LPVOID params)
{
	TRACE(L"newReset: cleaning-up.");
	
	g_bGotFormat = false;
	
    int i;
    /*
	for (i = 0; i<400; i++) {
		if (myFonts[i/100][i%100]) {
			myFonts[i/100][i%100]->OnLostDevice();
		}
	}
    */

	CALLCHAIN(hk_D3D_Reset, it) {
		PFNRESETPROC NextCall = (PFNRESETPROC)*it;
		NextCall(self, params);
	}
	
    if (!_noshade)
        InvalidateDeviceObjects(self);

	HRESULT res = g_orgReset(self, params);
	TRACE(L"newReset: Reset() is done. About to return.");
	
	for (i = 0; i<400; i++) {
        /*
		if (myFonts[i/100][i%100]) {
			myFonts[i/100][i%100]->OnResetDevice();
		}
        */
	}

	return res;
}

HRESULT STDMETHODCALLTYPE newSetTransform(IDirect3DDevice9* self, 
		D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	CALLCHAIN(hk_D3D_SetTransform, it) {
		PFNSETTRANSFORMPROC NextCall = (PFNSETTRANSFORMPROC)*it;
		NextCall(self, State, pMatrix);
	}

	HRESULT res = g_orgSetTransform(self, State, pMatrix);
    return res;
}

void kloadGetBackBufferInfo(IDirect3DDevice9* d3dDevice)
{
	TRACE(L"kloadGetBackBufferInfo: called.");

	IDirect3DSurface9* backBuffer;
	// get the 0th backbuffer.
	if (SUCCEEDED(d3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer)))
	{
		D3DSURFACE_DESC desc;
		backBuffer->GetDesc(&desc);
		
		g_pesinfo.bbWidth = desc.Width;
		g_pesinfo.bbHeight = desc.Height;
		g_pesinfo.stretchX = g_pesinfo.bbWidth/1024.0;
		g_pesinfo.stretchY = g_pesinfo.bbHeight/768.0;

		TRACE(L"kloadGetBackBufferInfo: got new back buffer format and info.");
		g_bGotFormat = true;
		
		// release backbuffer
		backBuffer->Release();

        // initialize vertext buffers
        if (!_noshade)
            RestoreDeviceObjects(d3dDevice);
	}
	
	return;
}

void prepareRenderPlayers() {
	
	// sometimes this doesn't work, should be moved to some other place
	// free this map if no edit mode player was rendered in the last frame
	if (false && freeEditModeData && g_editPlayerInfoMap.size() > 0) {
		for (g_editPlayerInfoMapIt = g_editPlayerInfoMap.begin(); 
				g_editPlayerInfoMapIt != g_editPlayerInfoMap.end();
				g_editPlayerInfoMapIt++)
			{
				HeapFree(GetProcessHeap(), 0, g_editPlayerInfoMapIt->second);
			}
		g_editPlayerInfoMap.clear();
	}
		
	freeEditModeData = false;
	bool freeEditModeDataSet = false;

	TexPlayerInfo tpi;
	EditPlayerInfo* epi = NULL;
	WORD orgTexIds[50];
	DWORD firstPlayerInfo[2], lastPlayerInfo[2];
	DWORD replayTeamInfo[2], firstReplayPlayerInfo[2], lastReplayPlayerInfo[2];
	DWORD playerInfo;
	
	DWORD generalInfo = *(DWORD*)(dta[GENERALINFO]);
	DWORD replayGeneralInfo = *(DWORD*)(dta[REPLAYGENERALINFO]);
	
	for (int i = 0; i < 2; i++) {
		if (generalInfo) {
			firstPlayerInfo[i] = *(DWORD*)(generalInfo + 0x150 + i*4);
			lastPlayerInfo[i] = firstPlayerInfo[i] + 32 * 0x1c0;
		}
		
		if (replayGeneralInfo) {
			replayTeamInfo[i] = replayGeneralInfo + 0x360 + i*0x4c;
			firstReplayPlayerInfo[i] = replayGeneralInfo + 0x3f8 + i*11*0x80;
			lastReplayPlayerInfo[i] = firstReplayPlayerInfo[i] + 11*0x80;
		}
	}
	
	DWORD* startVal = *(DWORD**)(dta[PLAYERDATA]);
	DWORD* nextVal = (DWORD*)*startVal;
	
	while (nextVal != startVal) {
		ZeroMemory(&tpi, sizeof(TexPlayerInfo));
		
		DWORD temp = *(nextVal + 2);
		nextVal = (DWORD*)*nextVal;
		
		if (!temp) continue;
		
		DWORD temp2 = *(DWORD*)(temp + 0x64);
		if (!temp2) continue;
		DWORD* texArr = *(DWORD**)(temp2 + 0x454);
		if (!texArr) continue;
		
		tpi.gameReplay = false;
		tpi.epiMode = EPIMODE_NO;
		epi = NULL;
		
		tpi.referee = (*(DWORD*)temp == dta[ISREFEREEADDR])?1:0;
		if (!tpi.referee) {
			// for players
			
			DWORD playerName = *(DWORD*)(temp + 0x534);
			tpi.playerName = (char*)playerName;
			
			tpi.team = 0xff;
			if (generalInfo) {
				if (playerName >= firstPlayerInfo[0] && playerName < lastPlayerInfo[0])  tpi.team = 0;
				if (playerName >= firstPlayerInfo[1] && playerName < lastPlayerInfo[1])  tpi.team = 1;
			}
			
			if (tpi.team == 0xff) {
				// try if it's replay mode
				if (replayGeneralInfo) {			
					if (playerName >= firstReplayPlayerInfo[0] && playerName < lastReplayPlayerInfo[0])  tpi.team = 0;
					if (playerName >= firstReplayPlayerInfo[1] && playerName < lastReplayPlayerInfo[1])  tpi.team = 1;
				}
				
				if (tpi.team == 0xff) {
					// or edit/kit selection mode?
					g_editPlayerInfoMapIt = g_editPlayerInfoMap.find(playerName - 0x6c);
					if (g_editPlayerInfoMapIt != g_editPlayerInfoMap.end()) {
						epi = g_editPlayerInfoMapIt->second;
						tpi.epiMode = epi->mode;
						
						freeEditModeData = false;
						freeEditModeDataSet = true;
						
					} else {	
						continue;
					}
				} else {
					tpi.gameReplay = true;
				}
			}
			
			if (!freeEditModeDataSet)
				freeEditModeData = true;
			
			if (!tpi.gameReplay && !tpi.epiMode) {
				tpi.playerInfo = playerName - 0xee;
				tpi.teamPos = (tpi.playerInfo - firstPlayerInfo[tpi.team]) / 0x1c0;
				tpi.lineupPos = tpi.teamPos; // later
				tpi.teamId = *(WORD*)(tpi.playerInfo + 0x11c);
				tpi.playerId = *(DWORD*)(tpi.playerInfo + 0x12c);
				tpi.isGk = (tpi.lineupPos % 11 == 0);
				
			} else if (tpi.gameReplay) {
				tpi.playerInfo = playerName - 0x6d;
				//tpi.teamPos = ...
				tpi.lineupPos = (tpi.playerInfo - firstReplayPlayerInfo[tpi.team]) / 0x80;
				tpi.teamId = *(WORD*)(replayTeamInfo[tpi.team]);
				//tpi.playerId = ...
				tpi.isGk = (tpi.lineupPos == 0);
				
			} else {
				tpi.team = epi->team;
				//tpi.playerInfo = ...
				tpi.teamPos = 
				tpi.lineupPos = epi->num;
				tpi.teamId = epi->teamId;
				tpi.playerId = epi->playerId;
				tpi.isGk = epi->isGk;
			}
			
			renderedPlayers += tpi.playerName;
			renderedPlayers += "\n";
			

		} else {
			// for referees
		}
		
		tpi.lod = *(BYTE*)(temp2 + 0x4c4);
		//

		for (int i = 0; i < 9; i++)  {
			DWORD coll = texArr[i];
			if (!coll) continue;
				
			DWORD temp3 = *(DWORD*)(coll + 0x444);
			if (!temp3) continue;
			
			DWORD ktmdl = *(DWORD*)(temp3 + 0x120);
			if (!ktmdl) continue;
			ktmdl = *(DWORD*)(ktmdl + 0xc);
			if (!ktmdl) continue;

			DWORD ktmdlEntriesStart = ktmdl + *(DWORD*)(ktmdl + 0x54);
			WORD* texTypes = (WORD*)(ktmdl + *(DWORD*)(ktmdl + 0x80) + 8);
			BYTE orgTexMaxNum = (*(DWORD*)(ktmdl + 0x60) - *(DWORD*)(ktmdl + 0x54)) / 0x50;

			for (int j = 0; j < orgTexMaxNum; j++) {
				WORD orgTexNum = *(WORD*)(ktmdlEntriesStart + 0x50 * j + 0xe);
				orgTexIds[j] = texTypes[orgTexNum * 8];
			}

			CALLCHAIN(hk_RenderPlayer, it) {
				RENDERPLAYER NextCall = (RENDERPLAYER)*it;
				NextCall(&tpi, coll, i, orgTexIds, orgTexMaxNum);
			}
		}
	}
}

KEXPORT WORD getOrgTexId(DWORD coll, DWORD num) {
	if (!coll) return 0;
		
	DWORD temp3 = *(DWORD*)(coll + 0x444);
	if (!temp3) return 0;
	
	DWORD ktmdl = *(DWORD*)(temp3 + 0x120);
	if (!ktmdl) return 0;
	ktmdl = *(DWORD*)(ktmdl + 0xc);
	if (!ktmdl) return 0;
		
	DWORD ktmdlEntriesStart = ktmdl + *(DWORD*)(ktmdl + 0x54);
	WORD* texTypes = (WORD*)(ktmdl + *(DWORD*)(ktmdl + 0x80) + 8);
	
	WORD orgTexNum = *(WORD*)(ktmdlEntriesStart + 0x50 * num + 0xe);
	
	return texTypes[orgTexNum * 8];	
	
}

KEXPORT void setTextureHeaderAddr(DWORD* p1, IDirect3DTexture9* tex)
{
	if (!p1) return;
	
	// save the value
	g_replacedHeadersIt = g_replacedHeaders.find(p1);
	// only save if this header hasn't been replaced yet
	if (g_replacedHeadersIt ==  g_replacedHeaders.end()) {
		g_replacedHeaders[p1] = *p1;
	}
	
	// create a buffer and set it as the new value
	DWORD value = (DWORD)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x24);
	*(DWORD*)(value + 0x20) = (DWORD)tex;
	*p1 = value;
	return;
}

KEXPORT void setNewSubTexture(DWORD coll, BYTE num, IDirect3DTexture9* tex)
{
	if (!coll) return;
		
	DWORD temp1 = *(DWORD*)(coll + 0x444);
	if (!temp1) return;
		
	DWORD loadedNum = *(DWORD*)(temp1 + 0x15c);

	g_replacedMap[temp1 ^ ((num & 0xff) * 0x1010101)] = tex;
		
	if (num > loadedNum - 1) return;
	
	DWORD* subtextures = *(DWORD**)(temp1 + 0x164);
	if (!subtextures) return;
	
	setTextureHeaderAddr(subtextures + num, tex);
	
	return;
}

KEXPORT IDirect3DTexture9* getSubTexture(DWORD coll, DWORD num)
{
	if (!coll) return NULL;
		
	DWORD temp1 = *(DWORD*)(coll + 0x444);
	if (!temp1) return NULL;
		
	DWORD loadedNum = *(DWORD*)(temp1 + 0x15c);
	if (num > loadedNum - 1) return NULL;
	
	DWORD** subtextures = *(DWORD***)(temp1 + 0x164);
	if (!subtextures) return NULL;
		
	DWORD* temp2 = *(subtextures + num);
	if (!temp2) return NULL;
	
	return (IDirect3DTexture9*)(*(temp2 + 8));
}
	

DWORD STDMETHODCALLTYPE hookedLoadTextureForPlayer(DWORD num, DWORD newTex)
{
	DWORD _ESI, _EBP;
	__asm mov _ESI, esi
	__asm mov _EBP, ebp

	DWORD* subtextures = *(DWORD**)(_ESI + 0x164);
	DWORD loadedNum = *(DWORD*)(_ESI + 0x15c);
	bool isNewTex = false;
	
	// free our own subtexture headers
	if (subtextures) {
		for (int i=0; i < loadedNum; i++) {
			g_replacedHeadersIt = g_replacedHeaders.find(subtextures + i);
			if (g_replacedHeadersIt != g_replacedHeaders.end()) {
				HeapFree(GetProcessHeap(), 0, (VOID*)*(g_replacedHeadersIt->first));
				*(g_replacedHeadersIt->first) = g_replacedHeadersIt->second;
				g_replacedHeaders.erase(subtextures + i);
			}
		}
	}
	
	DWORD newTex1 = newTex;
	
	g_replacedMapIt = g_replacedMap.find(_ESI ^ ((num & 0xff) * 0x1010101));
	if (g_replacedMapIt != g_replacedMap.end()) {
		// create a buffer and set it as the new value
		newTex1 = (DWORD)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x24);
		*(DWORD*)(newTex1 + 0x20) = (DWORD)g_replacedMapIt->second;
		isNewTex = true;
	}
		
	// call the original function
	__asm mov esi, _ESI
	DWORD res = ((LOADTEXTUREFORPLAYER)code[C_LOADTEXTUREFORPLAYER])(num, newTex1);
	
	
	subtextures = *(DWORD**)(_ESI + 0x164);
	loadedNum = *(DWORD*)(_ESI + 0x15c);

	if (subtextures) {
		for (int i=0; i < loadedNum; i++) {
			if (i != num) {
				g_replacedMapIt = g_replacedMap.find(_ESI ^ ((i & 0xff) * 0x1010101));
				if (g_replacedMapIt != g_replacedMap.end()) {
					setTextureHeaderAddr(subtextures + i, g_replacedMapIt->second);
				}
			} else if (isNewTex) {
				// save the value
				g_replacedHeadersIt = g_replacedHeaders.find(subtextures + num);
				// only save if this header hasn't been replaced yet
				if (g_replacedHeadersIt ==  g_replacedHeaders.end()) {
					g_replacedHeaders[subtextures + num] = newTex;
				}
			}
		}
	}

	return newTex1;
}

void hookedBeginRenderPlayer()
{
	DWORD _ECX;
	__asm mov _ECX, ecx
	
	if (!rpPrepared) {
		prepareRenderPlayers();
		rpPrepared = true;
	}
	
	// call the original function
	if ((*(DWORD*)_ECX + 0xc) == code[TBL_BEGINRENDER1]) {
		__asm mov ecx, _ECX
		g_orgBeginRender1();
	} else if ((*(DWORD*)_ECX + 0xc) == code[TBL_BEGINRENDER2]) {
		__asm mov ecx, _ECX
		g_orgBeginRender2();
	} else {
		TRACE(L"This should never happen!");
	}
	
	return;
}

DWORD STDMETHODCALLTYPE hookedEditCopyPlayerName(DWORD p1, DWORD p2)
{
	DWORD _ECX, _ESI, _EBX, _EBP, res;
	__asm {
		mov _ECX, ecx
		mov _ESI, esi
		mov _EBX, ebx
		mov _EBP, ebp
	}
	
	DWORD* orgParameters = (DWORD*)(_EBP + 0x28);
	// memory leak here!
	EditPlayerInfo* epi = (EditPlayerInfo*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(EditPlayerInfo));
	epi->mode = EPIMODE_EDITPLAYER;
	epi->playerId = orgParameters[1];
	if (epi->playerId == 0)
		if (*(DWORD*)(_EBX + 0x168) == 2)
			epi->mode = EPIMODE_KITSELECT;
		else
			epi->mode = EPIMODE_EDITTEAM;
	epi->teamId = orgParameters[2] & 0xffff;
	epi->num = orgParameters[3] & 0xff;
	epi->team = orgParameters[5] & 0xff;
	epi->isGk = (*(DWORD*)(_ESI + 0xc) == 0);
	g_editPlayerInfoMap[_ESI + 0x14] = epi;
	
	// the compiler would use ecx for the parameters...
	__asm {
		push p2
		push p1
		mov ecx, _ECX
		mov esi, _ESI
		mov ebx, _EBX
		call ds:[g_orgEditCopyPlayerName]
	}
	return res;
}

void Patch(void* addr, void* value, int type, int size)
{
    DWORD protection;
    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &protection);

    switch(type)
    {
        case DT_DWORD:
            *(DWORD*) addr = *(DWORD*) value;
            break;
        case DT_FLOAT:
            *(float*) addr = *(float*) value;
            break;
        case DT_DOUBLE:
            *(double*) addr = *(double*) value;
            break;
        default:
            break;
    }

    DWORD temp;
    VirtualProtect(addr, size, protection, &temp);
}

KEXPORT void PatchCode(DWORD addr, void* patch, size_t len)
{
	if (addr)
	{
	    BYTE* bptr = (BYTE*)addr;
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, (len+1)/4*4, newProtection, &protection)) {
            memcpy(bptr, patch, len);
	        TRACE(L"Patched code at address (%08x): %d bytes", addr, len);
	    }
        else {
            TRACE(L"FAILED to patch code at %08x", addr);
        }
	}
}

DWORD hookedCopyString(DWORD dest, DWORD destLen, DWORD src, DWORD srcLen)
{	
	DWORD _ESI;
	__asm mov _ESI, esi
	
	g_editPlayerInfoMapIt = g_editPlayerInfoMap.find(src);
	if (g_editPlayerInfoMapIt != g_editPlayerInfoMap.end()) {
		g_editPlayerInfoMap[dest] = g_editPlayerInfoMapIt->second;
	}
	
	return MasterCallNext(dest, destLen, src, srcLen);
}

KEXPORT DWORD HookIndirectCall(DWORD addr, void* func)
{
    DWORD target = (DWORD)func;
	if (addr && target)
	{
	    BYTE* bptr = (BYTE*)addr;
        DWORD* orgTarget = *(DWORD**)(bptr + 2);
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, 16, newProtection, &protection)) {
	        bptr[0] = 0xe8;
	        DWORD* ptr = (DWORD*)(addr + 1);
	        ptr[0] = target - (DWORD)(addr + 5);
            // padding with NOP
            bptr[5] = 0x90;
	        TRACE(L"Function (%08x) HOOKED at address (%08x)", target, addr);
            return *orgTarget;
	    }
	}
    return NULL;
}

KEXPORT void HookIndirectCall2(DWORD addr, void* func)
{
    DWORD target = (DWORD)func;
	if (addr && target)
	{
	    BYTE* bptr = (BYTE*)addr;
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, 16, newProtection, &protection)) {
	        bptr[0] = 0xe8;
	        DWORD* ptr = (DWORD*)(addr + 1);
	        ptr[0] = target - (DWORD)(addr + 5);
            // padding with NOP
            bptr[5] = 0x90;
	        TRACE(L"Function (%08x) HOOKED at address (%08x)", target, addr);
	    }
	}
}

KEXPORT void HookCallPoint(DWORD addr, void* func, int codeShift, int numNops, bool addRetn)
{
    DWORD target = (DWORD)func + codeShift;
	if (addr && target)
	{
	    BYTE* bptr = (BYTE*)addr;
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, 16, newProtection, &protection)) {
	        bptr[0] = 0xe8;
	        DWORD* ptr = (DWORD*)(addr + 1);
	        ptr[0] = target - (DWORD)(addr + 5);
            // padding with NOPs
            for (int i=0; i<numNops; i++) bptr[5+i] = 0x90;
            if (addRetn)
                bptr[5+numNops]=0xc3;
	        TRACE(L"Function (%08x) HOOKED at address (%08x)", target, addr);
	    }
        else
        {
	        LOG(L"FAILED to hook function (%08x) at (%08x)", target, addr);
        }
        TRACE(L"protection=%08x, newProtection=%08x", 
                protection, newProtection);
        if (!VirtualProtect(bptr, 16, protection, &newProtection)) {
            LOG(L"FAILED to restore protection at %p", bptr);
        }
	}
}

KEXPORT DWORD GetTargetAddress(DWORD addr)
{
	if (addr)
	{
	    BYTE* bptr = (BYTE*)addr;
	    DWORD protection = 0;
	    DWORD newProtection = PAGE_EXECUTE_READWRITE;
	    if (VirtualProtect(bptr, 8, newProtection, &protection)) 
        {
            // get original target
            DWORD* ptr = (DWORD*)(addr + 1);
            return (DWORD)(ptr[0] + addr + 5);
	    }
	}
    return 0;
}

KEXPORT int getOverlayPage()
{
    return g_overlayPage;
}

KEXPORT void setOverlayPageVisible(int page, bool flag)
{
    if (0 <= page && page < _numOverlayPages)
        _activeOverlayPages[page] = flag;
}

KEXPORT int addOverlayCallback(OVERLAY_EVENT_CALLBACK callback, bool ownPage)
{
    _overlay_callbacks.push_back(callback);
    if (ownPage)
        return _numOverlayPages++;
    return -1;
}

KEXPORT void addKeyboardCallback(KEY_EVENT_CALLBACK callback)
{
    _key_callbacks.push_back(callback);
}

KEXPORT void addMenuCallback(MENU_EVENT_CALLBACK callback)
{
    _menu_callbacks.push_back(callback);
}

/**
 * @param delta: either 1 (if menuMode increased), or -1 (if menuMode decreased)
 */
void hookTriggerSelectionOverlay(int delta)
{
    DWORD menuMode = *(DWORD*)dta[MENU_MODE_IDX];
    DWORD menuMode2 = *(DWORD*)(dta[MENU_MODE_IDX]+8);
    DWORD ind = *(DWORD*)dta[MAIN_SCREEN_INDICATOR];
    DWORD inGameInd = *(DWORD*)dta[INGAME_INDICATOR];
    DWORD cupModeInd = *(DWORD*)dta[CUP_MODE_PTR];

    // call the menu-mode change callbacks
    for (list<MENU_EVENT_CALLBACK>::iterator it = _menu_callbacks.begin();
            it != _menu_callbacks.end();
            it++)
        (*it)(delta, menuMode, ind, inGameInd, cupModeInd);

    // overlay triggering
    if (ind == 0 && inGameInd == 0 && menuMode == 2 && cupModeInd != 0)
    {
        if (!g_overlayOn)
        {
            g_overlayOn = true;
            TRACE(L"overlay ON (CUPS)");
            HookKeyboard();
            // reset overlay page
            ResetOverlayPage();
            // call the callbacks
            for (list<OVERLAY_EVENT_CALLBACK>::iterator it = _overlay_callbacks.begin();
                    it != _overlay_callbacks.end();
                    it++)
                (*it)(g_overlayOn, false, delta, menuMode);
        }
    }
    else if (cupModeInd != 0)
    {
        if (g_overlayOn)
        {
            TRACE(L"overlay OFF (CUPS)");
            g_overlayOn = false;
            UnhookKeyboard();
            // call the callbacks
            for (list<OVERLAY_EVENT_CALLBACK>::iterator it = _overlay_callbacks.begin();
                    it != _overlay_callbacks.end();
                    it++)
                (*it)(g_overlayOn, false, delta, menuMode);
        }
    }

    // Exhibition mode
    TRACE(L"menuMode = %x (delta=%d)",menuMode,delta);
    if ((menuMode == 0x21 && menuMode2 == 0x2a) 
            || (menuMode == 0x0d && menuMode2 == 0x0a))
    {
        if (!g_overlayOn)
        {
            g_overlayOn = true;
            TRACE(L"overlay ON");
            HookKeyboard();
            // reset overlay page
            ResetOverlayPage();
            // call the callbacks
            for (list<OVERLAY_EVENT_CALLBACK>::iterator it = _overlay_callbacks.begin();
                    it != _overlay_callbacks.end();
                    it++)
                (*it)(g_overlayOn, true, delta, menuMode);
            //if (menuMode == 0x21) __asm { int 3 }
        }
    }
    else if ((menuMode == 0x22 && delta == 1) 
            || (menuMode == 0x20 && delta == -1 && menuMode2 == 0x28)
            || (menuMode == 0x0a && menuMode2 == 0x04) 
            || (menuMode == 0xe && delta == 1)
            || (menuMode == 0xc && delta == -1))
    {
        if (g_overlayOn)
        {
            TRACE(L"overlay OFF");
            g_overlayOn = false;
            UnhookKeyboard();
            // call the callbacks
            for (list<OVERLAY_EVENT_CALLBACK>::iterator it = _overlay_callbacks.begin();
                    it != _overlay_callbacks.end();
                    it++)
                (*it)(g_overlayOn, true, delta, menuMode);
            //if (menuMode == 0x20) __asm { int 3 }
        }
    }
}

void hookAddMenuModeCallPoint()
{
    __asm 
    {
        pushfd
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov esi,g_menuMode
        add [esi],1 // execute replaced code
        push 0x00000001
        call hookTriggerSelectionOverlay
        add esp,4
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void hookSubMenuModeCallPoint()
{
    __asm 
    {
        pushfd
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov esi,g_menuMode
        sub [esi],1 // execute replaced code
        push 0xffffffff
        call hookTriggerSelectionOverlay
        add esp,4
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void enterUniformSelectCallPoint()
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
        call enterUniformSelect
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        cmp byte ptr ds:[esi+0x13c],0  //execute replaced code
        retn
    }
}

void exitUniformSelectCallPoint()
{
    __asm 
    {
        pushfd
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        call exitUniformSelect
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        add esp,4  // exit the call subroutine
        retn 4     // ...
    }
}

KEXPORT void enterUniformSelect()
{
    HookKeyboard();
    ResetOverlayPage();
    NextPage();

    // call the callbacks
    for (list<UNIFORM_SELECT_EVENT_CALLBACK>::iterator it = 
                _uniformSelectEventCallbacks.begin();
            it != _uniformSelectEventCallbacks.end();
            it++)
        (*it)(true);
}

KEXPORT void exitUniformSelect()
{
    UnhookKeyboard();
    ResetOverlayPage();

    // call the callbacks
    for (list<UNIFORM_SELECT_EVENT_CALLBACK>::iterator it = 
                _uniformSelectEventCallbacks.begin();
            it != _uniformSelectEventCallbacks.end();
            it++)
        (*it)(false);
}

void HookKeyboard()
{
    if (g_hKeyboardHook == NULL) 
    {
		g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, hInst, GetCurrentThreadId());
		LOG(L"Installed keyboard hook: g_hKeyboardHook = %d", (DWORD)g_hKeyboardHook);
	}
}

void UnhookKeyboard()
{
	if (g_hKeyboardHook != NULL) 
    {
		UnhookWindowsHookEx(g_hKeyboardHook);
		LOG(L"Keyboard hook uninstalled.");
		g_hKeyboardHook = NULL;
	}
}

void PreviousPage()
{
    if (_numOverlayPages==0)
        return;

    int page = g_overlayPage-1;
    if (page<0) page = _numOverlayPages-1;
    while (page != g_overlayPage && !_activeOverlayPages[page])
    {
        page--;
        if (page<0) page = _numOverlayPages-1;
    }
    g_overlayPage = page;
}

void NextPage()
{
    if (_numOverlayPages==0)
        return;

    int page = g_overlayPage+1;
    if (page>=_numOverlayPages) page = 0;
    while (page != g_overlayPage && !_activeOverlayPages[page])
    {
        page++;
        if (page>=_numOverlayPages) page = 0;
    }
    g_overlayPage = page;
}

LRESULT CALLBACK KeyboardProc(int code1, WPARAM wParam, LPARAM lParam)
{
	if (code1 >= 0 && code1==HC_ACTION && lParam & 0x80000000) {
        // handle page flips
        if (wParam == VK_PRIOR) { // page up
            PreviousPage();
            TRACE(L"g_overlayPage = %d", g_overlayPage);
        }
        else if (wParam == VK_NEXT) { // page down
            NextPage();
            TRACE(L"g_overlayPage = %d", g_overlayPage);
        }
        else
        {
            // call the callbacks
            for (list<KEY_EVENT_CALLBACK>::iterator it = _key_callbacks.begin();
                    it != _key_callbacks.end();
                    it++)
                (*it)(code1, wParam, lParam);
        }
    }	

	return CallNextHookEx(g_hKeyboardHook, code1, wParam, lParam);
}

KEXPORT void addReadEditDataCallback(READ_DATA_CALLBACK callback)
{
    _readEditDataCallbacks.push_back(callback);
}

KEXPORT void addReadReplayDataCallback(READ_DATA_CALLBACK callback)
{
    _readReplayDataCallbacks.push_back(callback);
}

KEXPORT void addReadBalDataCallback(READ_DATA_CALLBACK callback)
{
    _readBalDataCallbacks.push_back(callback);
}

KEXPORT void addWriteEditDataCallback(WRITE_DATA_CALLBACK callback)
{
    _writeEditDataCallbacks.push_back(callback);
}

KEXPORT void addWriteReplayDataCallback(WRITE_DATA_CALLBACK callback)
{
    _writeReplayDataCallbacks.push_back(callback);
}

KEXPORT void addWriteBalDataCallback(WRITE_DATA_CALLBACK callback)
{
    _writeBalDataCallbacks.push_back(callback);
}

void hookPrepareDataToWriteCallPoint()
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
        mov ebx,dword ptr ss:[esp+0x2c]
        push ebx  // param: numBytes
        push eax  // param: dest
        call hookPrepareDataToWrite
        add esp,8 // pop params
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov ecx,dword ptr ds:[edi+0x24] // replaced code
        push ebx
        mov ebx,[esp+4]
        mov [esp+0x10],ebx
        pop ebx
        add esp,0x0c
        retn
    }
}

void hookPrepareDataToWrite(LPVOID buffer, DWORD numBytes)
{
    LOG(L"Preparing data for saving (at %08x, %08x bytes)...", 
        (DWORD)buffer, numBytes);

    if (numBytes == 0x76a678) {
        LOG(L"EDIT data to save...");

        // call the callbacks
        list<WRITE_DATA_CALLBACK>::iterator it;
        for (it = _writeEditDataCallbacks.begin();
                it != _writeEditDataCallbacks.end();
                it++)
            (*it)(buffer, numBytes);
    }
    else if (numBytes == 0x40090) {
        LOG(L"OPTION data to save...");
    }
    else if (numBytes == 0x791968) {
        LOG(L"BAL data to save...");

        // call the callbacks
        list<WRITE_DATA_CALLBACK>::iterator it;
        for (it = _writeBalDataCallbacks.begin();
                it != _writeBalDataCallbacks.end();
                it++)
            (*it)(buffer, numBytes);
    }
    else if (numBytes == 0x483c10) {
        LOG(L"REPLAY data to save...");

        // call the callbacks
        list<WRITE_DATA_CALLBACK>::iterator it;
        for (it = _writeReplayDataCallbacks.begin();
                it != _writeReplayDataCallbacks.end();
                it++)
            (*it)(buffer, numBytes);
    }
}

void hookAfterReadDataCallPoint()
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
        mov eax,edi
        add eax,0x20
        mov ebx,dword ptr ds:[edi+4]
        push ebx  // param: numBytes
        push eax  // param: buffer
        call hookAfterReadData
        add esp,8 // pop params
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        sub esp,8  // replaced code + stack manipulations
        push eax
        mov eax,[esp+0x0c]
        mov [esp+4],eax
        mov [esp+0x0c],edx
        add esi,0x2c
        mov [esp+0x08],esi
        pop eax
        retn
    }
}

void hookAfterReadData(LPVOID buffer, DWORD numBytes)
{
    LOG(L"Read and decrypted data (at %08x, %08x bytes)...", 
        (DWORD)buffer, numBytes);

    if (numBytes == 0x76a678) {
        LOG(L"EDIT data to load...");

        // call the callbacks
        list<WRITE_DATA_CALLBACK>::iterator it;
        for (it = _readEditDataCallbacks.begin();
                it != _readEditDataCallbacks.end();
                it++)
            (*it)(buffer, numBytes);
    }
    else if (numBytes == 0x40090) {
        LOG(L"OPTION data to load...");
    }
    else if (numBytes == 0x483c10) {
        LOG(L"REPLAY data to load...");

        // call the callbacks
        list<WRITE_DATA_CALLBACK>::iterator it;
        for (it = _readReplayDataCallbacks.begin();
                it != _readReplayDataCallbacks.end();
                it++)
            (*it)(buffer, numBytes);
    }
    else if (numBytes == 0x791968) {
        LOG(L"BAL data to load...");

        // call the callbacks
        list<WRITE_DATA_CALLBACK>::iterator it;
        for (it = _readBalDataCallbacks.begin();
                it != _readBalDataCallbacks.end();
                it++)
            (*it)(buffer, numBytes);
    }

    // adjust CRC32 checksum
    DWORD* pChecksum = (DWORD*)((BYTE*)buffer-0x18);
    LOG(L"crc was: %08x", *pChecksum);
    *pChecksum = 0;
    *pChecksum = GetCRC((BYTE*)buffer-0x20, numBytes+0x20);
    LOG(L"crc now: %08x", *pChecksum);
}

/**
 * WriteFile interceptor
 */
BOOL WINAPI hookWriteFile(
  HANDLE hFile,
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToWrite,
  LPDWORD lpNumberOfBytesWritten,
  LPOVERLAPPED lpOverlapped
)
{
    TRACE(L"WriteFile: len=%08x", nNumberOfBytesToWrite);
    if (nNumberOfBytesToWrite == dta[EDIT_DATA_SIZE])  // edit data
    {
        LOG(L"Saving Edit Data...");

        // call the callbacks
        for (list<WRITE_DATA_CALLBACK>::iterator it = _writeEditDataCallbacks.begin();
                it != _writeEditDataCallbacks.end();
                it++)
            (*it)(lpBuffer, nNumberOfBytesToWrite);

        // adjust CRC32 checksum
        DWORD* pChecksum = (DWORD*)((BYTE*)lpBuffer + 0x188);
        *pChecksum = 0;
        *pChecksum = GetCRC((BYTE*)lpBuffer + 0x180, 
                dta[EDIT_DATA_SIZE] - 0x180);
    }

    else if (nNumberOfBytesToWrite == dta[REPLAY_DATA_SIZE])  // replay data
    {
        LOG(L"Saving Replay Data...");

        // call the callbacks
        for (list<WRITE_DATA_CALLBACK>::iterator it = _writeReplayDataCallbacks.begin();
                it != _writeReplayDataCallbacks.end();
                it++)
            (*it)(lpBuffer, nNumberOfBytesToWrite);

        // adjust CRC32 checksum
        DWORD* pChecksum = (DWORD*)((BYTE*)lpBuffer + 0x188);
        *pChecksum = 0;
        *pChecksum = GetCRC((BYTE*)lpBuffer + 0x180, 
                dta[REPLAY_DATA_SIZE] - 0x180);
    }
    else if (nNumberOfBytesToWrite == dta[BAL_DATA_SIZE])  // BAL data
    {
        LOG(L"Saving BAL Data...");

        // call the callbacks
        for (list<WRITE_DATA_CALLBACK>::iterator it = _writeBalDataCallbacks.begin();
                it != _writeBalDataCallbacks.end();
                it++)
            (*it)(lpBuffer, nNumberOfBytesToWrite);

        // adjust CRC32 checksum
        DWORD* pChecksum = (DWORD*)((BYTE*)lpBuffer + 0x188);
        *pChecksum = 0;
        *pChecksum = GetCRC((BYTE*)lpBuffer + 0x180, 
                dta[BAL_DATA_SIZE] - 0x180);
    }

    //BOOL result = _writeFile(
    BOOL result = WriteFile(
            hFile,
            lpBuffer,
            nNumberOfBytesToWrite,
            lpNumberOfBytesWritten,
            lpOverlapped);

    if (result && nNumberOfBytesToWrite == dta[EDIT_DATA_SIZE])
        LOG(L"Edit Data SAVED.");
    else if (result && nNumberOfBytesToWrite == dta[REPLAY_DATA_SIZE])
        LOG(L"Replay Data SAVED.");
    else if (result && nNumberOfBytesToWrite == dta[BAL_DATA_SIZE])
        LOG(L"BAL Data SAVED.");

    return result;
}

/**
 * ReadFile interceptor
 */
BOOL WINAPI hookReadFile(
  HANDLE hFile,
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToRead,
  LPDWORD lpNumberOfBytesRead,
  LPOVERLAPPED lpOverlapped
)
{
    LOG(L"ReadFile: len=%08x", nNumberOfBytesToRead);

    //BOOL result = _readFile(
    BOOL result = ReadFile(
            hFile,
            lpBuffer,
            nNumberOfBytesToRead,
            lpNumberOfBytesRead,
            lpOverlapped);

    LOG(L"ReadFile DONE: bytesToRead=%08x, bytesRead=%08x, lpOverlapped=%p",
            nNumberOfBytesToRead, 
            (lpNumberOfBytesRead)?(*lpNumberOfBytesRead):0, 
            lpOverlapped);

    if (!result)
        return result;  // failed to read: return quickly.

    if (nNumberOfBytesToRead == dta[EDIT_DATA_SIZE])  // edit data
    {
        LOG(L"Loading Edit Data...");

        // call the callbacks
        for (list<READ_DATA_CALLBACK>::iterator it = _readEditDataCallbacks.begin();
                it != _readEditDataCallbacks.end();
                it++)
            (*it)(lpBuffer, nNumberOfBytesToRead);

        // adjust CRC32 checksum
        DWORD* pChecksum = (DWORD*)((BYTE*)lpBuffer + 0x188);
        *pChecksum = 0;
        *pChecksum = GetCRC((BYTE*)lpBuffer + 0x180, 
                dta[EDIT_DATA_SIZE] - 0x180);
    }

    else if (nNumberOfBytesToRead == dta[REPLAY_DATA_SIZE])  // replay data
    {
        LOG(L"Loading Replay Data...");

        // call the callbacks
        for (list<READ_DATA_CALLBACK>::iterator it = _readReplayDataCallbacks.begin();
                it != _readReplayDataCallbacks.end();
                it++)
            (*it)(lpBuffer, nNumberOfBytesToRead);

        // adjust CRC32 checksum
        DWORD* pChecksum = (DWORD*)((BYTE*)lpBuffer + 0x188);
        *pChecksum = 0;
        *pChecksum = GetCRC((BYTE*)lpBuffer + 0x180, 
                dta[REPLAY_DATA_SIZE] - 0x180);
    }

    else if (nNumberOfBytesToRead == dta[BAL_DATA_SIZE])  // BAL data
    {
        LOG(L"Loading BAL Data...");

        // call the callbacks
        for (list<READ_DATA_CALLBACK>::iterator it = _readBalDataCallbacks.begin();
                it != _readBalDataCallbacks.end();
                it++)
            (*it)(lpBuffer, nNumberOfBytesToRead);

        // adjust CRC32 checksum
        DWORD* pChecksum = (DWORD*)((BYTE*)lpBuffer + 0x188);
        *pChecksum = 0;
        *pChecksum = GetCRC((BYTE*)lpBuffer + 0x180, 
                dta[BAL_DATA_SIZE] - 0x180);
    }

    if (result && nNumberOfBytesToRead == dta[EDIT_DATA_SIZE])
        LOG(L"Edit Data LOADED.");
    else if (result && nNumberOfBytesToRead == dta[REPLAY_DATA_SIZE])
        LOG(L"Replay Data LOADED.");
    else if (result && nNumberOfBytesToRead == dta[BAL_DATA_SIZE])
        LOG(L"BAL Data LOADED.");

    return result;
}

KEXPORT void addUniformSelectCallback(
        UNIFORM_SELECT_EVENT_CALLBACK callback)
{
    _uniformSelectEventCallbacks.push_back(callback);
}

KEXPORT void addCopyPlayerDataCallback(COPY_PLAYER_DATA_CALLBACK callback)
{
    _copyPlayerDataCallbacks.push_back(callback);
}

void hookCopyPlayerData(PLAYER_INFO* players, DWORD numBytes, int place, bool writeList)
{
    // call the callbacks
    for (list<COPY_PLAYER_DATA_CALLBACK>::iterator it = _copyPlayerDataCallbacks.begin();
            it != _copyPlayerDataCallbacks.end();
            it++)
        (*it)(players, numBytes, place, writeList);
}

void hookAtCopyEditData(DWORD dest, DWORD numBytes, int place)
{
    LOG(L"hootAtCopyEditData CALLED!");
    DWORD* pData = (DWORD*)dta[PLAYERDATA];
    if (pData && *pData == dest-8) {
        // copying player data
        LOG(L"data copy: player data loaded (place=%d, numBytes=%08x)", 
            place, numBytes);
        hookCopyPlayerData((PLAYER_INFO*)dest, numBytes, place);
    }
}

void hookAtCopyEditDataCallPoint1()
{
    __asm {
        mov eax,dword ptr ss:[esp+0x0c]
        push eax
        mov eax,dword ptr ss:[esp+0x0c]
        push eax
        mov eax,dword ptr ss:[esp+0x0c]
        push eax
        call _org_copy
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        mov ebx,dword ptr ss:[esp+0x38]
        push 1    // param: place
        push ebx  // param: numBytes
        push eax  // param: dest
        call hookAtCopyEditData
        add esp,0x0c // pop params
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        add esp,0x0c
        retn
    }
}

void hookAtCopyEditDataCallPoint2()
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
        push 2    // param: place
        push 0    // param: numBytes
        push eax  // param: dest
        call hookAtCopyEditData
        add esp,0x0c // pop params
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        push eax
        mov eax,dword ptr ss:[esp+4] // move return address
        mov dword ptr ss:[esp+0x10],eax
        pop eax
        add esp,0x0c                  // execute replaced code
        cmp eax,dword ptr ss:[esi+8]  // ...
        retn
    }
}

KEXPORT void addReadNamesCallback(READ_NAMES_CALLBACK callback)
{
    _readNamesCallbacks.push_back(callback);
}

void hookAfterReadNamesCallPoint()
{
    __asm {
        mov [esi+0x13e],eax // replaced code
        pushfd 
        push ebp
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        call hookAfterReadNames
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void hookAfterReadNamesCallPoint2()
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
        cmp eax,0
        je skip
        call hookAfterReadNames
skip:   pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        push ebx
        mov ebx,[esp+4]
        mov [esp+0x10],ebx
        pop ebx
        add esp,0x0c
        cmp eax, dword ptr ds:[esi+8] // replaced code
        retn
    }
}

void hookAfterReadNames()
{
    // call the callbacks
    list<READ_NAMES_CALLBACK>::iterator it;
    for (it = _readNamesCallbacks.begin();
            it != _readNamesCallbacks.end();
            it++)
        (*it)();
}

