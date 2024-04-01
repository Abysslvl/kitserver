// hook.h
#ifndef _HOOK_
#define _HOOK_

#include "d3d9types.h"
#include "d3d9.h"
//#include "d3dx9tex.h"
#include "player.h"


enum HOOKS {
	hk_D3D_Create,
	hk_D3D_CreateDevice,
	hk_D3D_Present,
	hk_D3D_Reset,
	hk_D3D_SetTransform,
	hk_RenderPlayer,
	
	hk_LAST,
};

#define KDT_BOLD 0x01
#define KDT_ITALIC 0x02
KEXPORT void KDrawTextAbsolute(const wchar_t* str, UINT x, UINT y, D3DCOLOR color = 0xff000000, float fontSize = 16.0f, BYTE attr = KDT_BOLD, DWORD format = DT_LEFT);
KEXPORT void KDrawText(const wchar_t* str, UINT x, UINT y, D3DCOLOR color = 0xff000000, float fontSize = 16.0f, BYTE attr = KDT_BOLD, DWORD format = DT_LEFT);

KEXPORT void hookFunction(HOOKS h, void* addr);
KEXPORT void unhookFunction(HOOKS h, void* addr);
void initAddresses();

void hookDirect3DCreate9();
void hookOthers();


// IDirect3D9
#define VTAB_CREATEDEVICE 16
// IDirect3DDevice9
#define VTAB_RESET 16
#define VTAB_PRESENT 17
#define VTAB_SETTRANSFORM 44

typedef struct _TexPlayerInfo {
	BYTE referee;
	BYTE lod;
	bool gameReplay;
	BYTE epiMode;
	DWORD playerInfo;
	char* playerName;
	BYTE team; // 0 or 1
	BYTE teamPos; // 0 to 31
	BYTE lineupPos; // 0 to 10, maybe 11 on substitutions
	WORD teamId;
	DWORD playerId;
	bool isGk;
		
} TexPlayerInfo;

enum {EPIMODE_NO, EPIMODE_EDITPLAYER, EPIMODE_KITSELECT, EPIMODE_EDITTEAM};
typedef struct _EditPlayerInfo {
	BYTE mode;
	WORD teamId;
	DWORD playerId;
	bool isGk;
	BYTE num;
	BYTE team;
			
} EditPlayerInfo;


typedef void   (*ALLVOID)();
typedef DWORD  (STDMETHODCALLTYPE *LOADTEXTUREFORPLAYER)(DWORD, DWORD);
typedef void   (*RENDERPLAYER)(TexPlayerInfo* tpi, DWORD coll, DWORD num, WORD* orgTexIds, BYTE orgTexMaxNum);

typedef IDirect3D9* (STDMETHODCALLTYPE *PFNDIRECT3DCREATE9PROC)(UINT sdkVersion);
typedef HRESULT (STDMETHODCALLTYPE *PFNCREATEDEVICEPROC)(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);
typedef HRESULT (STDMETHODCALLTYPE *PFNPRESENTPROC)(IDirect3DDevice9* self, 
		CONST RECT* src, CONST RECT* dest, HWND hWnd, LPVOID unused);
typedef HRESULT (STDMETHODCALLTYPE *PFNRESETPROC)(IDirect3DDevice9* self, LPVOID);
typedef HRESULT (STDMETHODCALLTYPE *PFNSETTRANSFORMPROC)(IDirect3DDevice9* self, 
		D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);

IDirect3D9* STDMETHODCALLTYPE newDirect3DCreate9(UINT sdkVersion);
HRESULT STDMETHODCALLTYPE newCreateDevice(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters, 
    IDirect3DDevice9** ppReturnedDeviceInterface);
KEXPORT IDirect3DDevice9* getActiveDevice();
HRESULT STDMETHODCALLTYPE newPresent(IDirect3DDevice9* self, CONST RECT* src, CONST RECT* dest,
	HWND hWnd, LPVOID unused);
HRESULT STDMETHODCALLTYPE newReset(IDirect3DDevice9* self, LPVOID params);
HRESULT STDMETHODCALLTYPE newSetTransform(IDirect3DDevice9* self, 
		D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
void kloadGetBackBufferInfo(IDirect3DDevice9* d3dDevice);


void prepareRenderPlayers();
KEXPORT void setTextureHeaderAddr(DWORD* p1, IDirect3DTexture9* tex);
KEXPORT void setNewSubTexture(DWORD coll, BYTE num, IDirect3DTexture9* tex);
KEXPORT IDirect3DTexture9* getSubTexture(DWORD coll, DWORD num);
KEXPORT WORD getOrgTexId(DWORD coll, DWORD num);
DWORD STDMETHODCALLTYPE hookedLoadTextureForPlayer(DWORD num, DWORD newTex);
void hookedBeginRenderPlayer();
DWORD STDMETHODCALLTYPE hookedEditCopyPlayerName(DWORD p1, DWORD p2);
DWORD hookedCopyString(DWORD dest, DWORD destLen, DWORD src, DWORD srcLen);

KEXPORT DWORD HookIndirectCall(DWORD addr, void* func);
KEXPORT void HookIndirectCall2(DWORD addr, void* func);
KEXPORT void HookCallPoint(DWORD addr, void* func, int codeShift, int numNops, bool addRetn=false);
KEXPORT DWORD GetTargetAddress(DWORD addr);
KEXPORT void Patch(void* addr, void* value, int type, int size);
KEXPORT void PatchCode(DWORD addr, void* patch, size_t len);

typedef void (*OVERLAY_EVENT_CALLBACK)(bool overlayOn, bool isExhibitionMode, int delta, DWORD menuMode);
KEXPORT int addOverlayCallback(OVERLAY_EVENT_CALLBACK callback, bool ownPage);
KEXPORT int getOverlayPage();
KEXPORT void setOverlayPageVisible(int page, bool flag);
typedef void (*KEY_EVENT_CALLBACK)(int code, WPARAM wParam, LPARAM lParam);
KEXPORT void addKeyboardCallback(KEY_EVENT_CALLBACK callback);

typedef void (*WRITE_DATA_CALLBACK)(LPCVOID data, DWORD size);
typedef void (*READ_DATA_CALLBACK)(LPCVOID data, DWORD size);
KEXPORT void addWriteEditDataCallback(WRITE_DATA_CALLBACK callback);
KEXPORT void addWriteReplayDataCallback(WRITE_DATA_CALLBACK callback);
KEXPORT void addWriteBalDataCallback(WRITE_DATA_CALLBACK callback);
KEXPORT void addReadEditDataCallback(READ_DATA_CALLBACK callback);
KEXPORT void addReadReplayDataCallback(READ_DATA_CALLBACK callback);
KEXPORT void addReadBalDataCallback(READ_DATA_CALLBACK callback);

typedef void (*MENU_EVENT_CALLBACK)(int delta, DWORD menuMode, DWORD ind, DWORD inGameInd, DWORD cupModeInd);
KEXPORT void addMenuCallback(MENU_EVENT_CALLBACK callback);

typedef void (*COPY_PLAYER_DATA_CALLBACK)(
    PLAYER_INFO* players, DWORD numBytes, int place, bool writeList);
KEXPORT void addCopyPlayerDataCallback(COPY_PLAYER_DATA_CALLBACK callback);

typedef void (*UNIFORM_SELECT_EVENT_CALLBACK)(bool enter);
KEXPORT void addUniformSelectCallback(UNIFORM_SELECT_EVENT_CALLBACK callback);

typedef void (*READ_NAMES_CALLBACK)();
KEXPORT void addReadNamesCallback(READ_NAMES_CALLBACK callback);
#endif
