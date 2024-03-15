// lodmixer.cpp
#define UNICODE
#define THISMOD &k_lodmixer

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "kload_exp.h"
#include "lodmixer.h"
#include "lodmixer_addr.h"
#include "dllinit.h"

#define FLOAT_ZERO 0.0001f
#define SET(p,v) {if (v>FLOAT_ZERO) {*(p) = v;}}

KMOD k_lodmixer={MODID,NAMELONG,NAMESHORT,DEFAULT_DEBUG};

HINSTANCE hInst;
LMCONFIG _lmconfig = {
    {DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_ASPECT_RATIO}, 
    {
        DEFAULT_LOD_1,
        DEFAULT_LOD_2,
        DEFAULT_LOD_3,
        DEFAULT_LOD_4,
        DEFAULT_LOD_5,
        DEFAULT_LOD_6,
        DEFAULT_LOD_7,
        DEFAULT_LOD_8,
        DEFAULT_LOD_9,
        DEFAULT_LOD_10,
        DEFAULT_LOD_11,
        DEFAULT_LOD_12,
        DEFAULT_LOD_13,
        DEFAULT_LOD_14,
        DEFAULT_LOD_15,
        DEFAULT_LOD_16,
        DEFAULT_LOD_17,
        DEFAULT_LOD_18,
        DEFAULT_LOD_19,
        DEFAULT_LOD_20,
    },
    DEFAULT_ASPECT_RATIO_CORRECTION_ENABLED,
    DEFAULT_CONTROLLER_CHECK_ENABLED,
    DEFAULT_LODCHECK1,
    DEFAULT_PICTURE_QUALITY,
};

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);

void initLodMixer();
void lodmixerConfig(char* pName, const void* pValue, DWORD a);
void modifySettings();
BYTE getLB();
void getResolution(DWORD& width, DWORD& height);
void setResolution(DWORD width, DWORD height);
void setAspectRatio(float aspectRatio, bool manual);
void lodAtModeCheckCallPoint();
void lodAtModeCheckCallPoint2010();
void lodAtSettingsReadPoint();
void lodAtSettingsResetPoint();
KEXPORT DWORD lodAtModeCheck(DWORD mode);

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst=hInstance;
		RegisterKModule(&k_lodmixer);

		if (!checkGameVersion()) {
			LOG(L"Sorry, your game version isn't supported!");
			return false;
		}

		copyAdresses();
		hookFunction(hk_D3D_Create, initLodMixer);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		LOG(L"Detaching dll...");
	}
	return true;
}

void modifySettings()
{
    setResolution(_lmconfig.screen.width, _lmconfig.screen.height);
    if (_lmconfig.aspectRatioCorrectionEnabled)
    {
        DWORD width = 0, height = 0;
        getResolution(width,height);

        bool isManual = _lmconfig.screen.aspectRatio > FLOAT_ZERO;
        if (isManual) {
            float ar = _lmconfig.screen.aspectRatio;
            setAspectRatio(ar, true);
        }
        else {
            BYTE lb = getLB();
            if (lb & 0x01) { 
                LOG(L"Letter-boxing detected. "
                    L"Automatic AR-correction disabled");
            }
            else {
                float ar = float(width) / float(height);
                setAspectRatio(ar, false);
            }
        }
    }


    if (_lmconfig.pictureQuality != -1 && dta[WIDESCREEN_FLAG]) {
        // Enforce picture quality
        DWORD* pPictureQuality = (DWORD*)(dta[WIDESCREEN_FLAG]+4);
        *pPictureQuality = _lmconfig.pictureQuality;

        // disable quality check 1
        BYTE* pCode = (BYTE*)code[C_QUALITY_CHECK];
        if (pCode) {
            DWORD protection;
            DWORD newProtection = PAGE_EXECUTE_READWRITE;

            if (VirtualProtect(pCode, 4, newProtection, &protection)) {
                *(pCode+1) = 0xc0;  
            }
            else {
                LOG(L"Unable to disable quality check #1.");
            }
        }

        // disable quality check 2
        pCode = (BYTE*)code[C_QUALITY_CHECK_2];
        if (pCode) {
            DWORD protection;
            DWORD newProtection = PAGE_EXECUTE_READWRITE;

            if (VirtualProtect(pCode, 8, newProtection, &protection)) {
                memcpy(pCode, "\x33\xc0\x90\x90\x90", 5);
            }
            else {
                LOG(L"Unable to disable quality check #2.");
            }
        }
    }

    // LOD modifications
    {
        float* fptr =(float*)dta[LOD_PLAYERS_TABLE1];
        if (fptr)
        {
            SET(fptr+0, _lmconfig.lod.lodPlayersMiscS1);
            SET(fptr+1, _lmconfig.lod.lodPlayersMiscS2);
            SET(fptr+2, _lmconfig.lod.lodPlayersMiscS3);
            SET(fptr+3, _lmconfig.lod.lodPlayersReplayS1);
            SET(fptr+4, _lmconfig.lod.lodPlayersReplayS2);
            SET(fptr+5, _lmconfig.lod.lodPlayersReplayS3);
            // active player: corner kicks
            SET(fptr+6, _lmconfig.lod.lodActivePlayerCKs1); 
            SET(fptr+7, _lmconfig.lod.lodActivePlayerCKs2); 
            SET(fptr+8, _lmconfig.lod.lodActivePlayerCKs3); 
            // active player: free kicks
            SET(fptr+9, _lmconfig.lod.lodActivePlayerFKs1); 
            SET(fptr+10, _lmconfig.lod.lodActivePlayerFKs2); 
            SET(fptr+11, _lmconfig.lod.lodActivePlayerFKs3); 
        }

        fptr = (float*)dta[LOD_PLAYERS_TABLE2];
        if (fptr)
        {
            SET(fptr-3, _lmconfig.lod.lodPlayersEntranceS1);
            SET(fptr-2, _lmconfig.lod.lodPlayersEntranceS2);
            SET(fptr-1, _lmconfig.lod.lodPlayersEntranceS3);
            SET(fptr+0, _lmconfig.lod.lodPlayersEntranceS1);
            SET(fptr+1, _lmconfig.lod.lodPlayersEntranceS2);
            SET(fptr+2, _lmconfig.lod.lodPlayersEntranceS3);

            SET(fptr+3, _lmconfig.lod.lodPlayersInplayS1);
            SET(fptr+4, _lmconfig.lod.lodPlayersInplayS2);
            SET(fptr+5, _lmconfig.lod.lodPlayersInplayS3);
        }

        fptr = (float*)dta[LOD_REF_TABLE1];
        if (fptr) SET(fptr, _lmconfig.lod.lodRefReplay);
        fptr = (float*)dta[LOD_REF_TABLE2];
        if (fptr) SET(fptr, _lmconfig.lod.lodRefInplay);

        LOG(L"LOD levels set");
    }
}

void initLodMixer()
{
    // skip the settings-check call
    LOG(L"Initializing LOD mixer...");

    getConfig("lodmixer", "screen.width", DT_DWORD, 1, lodmixerConfig);
    getConfig("lodmixer", "screen.height", DT_DWORD, 2, lodmixerConfig);
    getConfig("lodmixer", "screen.aspect-ratio", DT_FLOAT, 3, lodmixerConfig);
    getConfig("lodmixer", "aspect-ratio.correction.enabled", DT_DWORD, 6, lodmixerConfig);
    getConfig("lodmixer", "controller.check.disabled", DT_DWORD, 7, lodmixerConfig);
    getConfig("lodmixer", "lod.check1", DT_DWORD, 8, lodmixerConfig);
    LOG(L"Screen resolution to force: %dx%d", 
            _lmconfig.screen.width, _lmconfig.screen.height);

    getConfig("lodmixer", "lod.players.entrance.s1", DT_FLOAT, 9,lodmixerConfig);
    getConfig("lodmixer", "lod.players.entrance.s2", DT_FLOAT,10,lodmixerConfig);
    getConfig("lodmixer", "lod.players.entrance.s3", DT_FLOAT,21,lodmixerConfig);
    getConfig("lodmixer", "lod.players.inplay.s1", DT_FLOAT,11,lodmixerConfig);
    getConfig("lodmixer", "lod.players.inplay.s2", DT_FLOAT,12,lodmixerConfig);
    getConfig("lodmixer", "lod.players.inplay.s3", DT_FLOAT,13,lodmixerConfig);
    getConfig("lodmixer", "lod.players.misc.s1", DT_FLOAT, 14,lodmixerConfig);
    getConfig("lodmixer", "lod.players.misc.s2", DT_FLOAT, 15,lodmixerConfig);
    getConfig("lodmixer", "lod.players.misc.s3", DT_FLOAT, 22,lodmixerConfig);
    getConfig("lodmixer", "lod.players.replay.s1", DT_FLOAT, 16,lodmixerConfig);
    getConfig("lodmixer", "lod.players.replay.s2", DT_FLOAT, 17,lodmixerConfig);
    getConfig("lodmixer", "lod.players.replay.s3", DT_FLOAT, 18,lodmixerConfig);

    getConfig("lodmixer", "lod.ref.inplay", DT_FLOAT, 19,lodmixerConfig);
    getConfig("lodmixer", "lod.ref.replay", DT_FLOAT, 20,lodmixerConfig);

    getConfig("lodmixer", "lod.active.player.ck.s1", DT_FLOAT, 24,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.ck.s2", DT_FLOAT, 25,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.ck.s3", DT_FLOAT, 26,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.fk.s1", DT_FLOAT, 27,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.fk.s2", DT_FLOAT, 28,
            lodmixerConfig);
    getConfig("lodmixer", "lod.active.player.fk.s3", DT_FLOAT, 29,
            lodmixerConfig);

    getConfig("lodmixer", "picture.quality", DT_DWORD, 23, lodmixerConfig);

    HookCallPoint(
        code[C_SETTINGS_READ], lodAtSettingsReadPoint, 6, 1, false);


    LOG(L"Initialization complete.");
    unhookFunction(hk_D3D_Create, initLodMixer);
}

void lodmixerConfig(char* pName, const void* pValue, DWORD a)
{
    int pq = DEFAULT_PICTURE_QUALITY;

	switch (a) {
		case 1:	// width
			_lmconfig.screen.width = *(DWORD*)pValue;
			break;
		case 2: // height
			_lmconfig.screen.height = *(DWORD*)pValue;
			break;
		case 3: // aspect ratio
			_lmconfig.screen.aspectRatio = *(float*)pValue;
			break;
		case 6: // LOD
			_lmconfig.aspectRatioCorrectionEnabled = *(DWORD*)pValue != 0;
			break;
		case 7: // Controller check
			_lmconfig.controllerCheckEnabled = *(DWORD*)pValue != 0;
			break;
        case 8: // lod-check
            _lmconfig.lodCheck1 = *(DWORD*)pValue != 0;
            break;

        case 9: // lod.players.entrance.s1
            _lmconfig.lod.lodPlayersEntranceS1 = *(float*)pValue;
            LOG(L"lod.players.entrance.s1 = %03f", 
                    _lmconfig.lod.lodPlayersEntranceS1);
            break;
        case 10: // lod.players.entrance.s2
            _lmconfig.lod.lodPlayersEntranceS2 = *(float*)pValue;
            LOG(L"lod.players.entrance.s2 = %03f", 
                    _lmconfig.lod.lodPlayersEntranceS2);
            break;
        case 11: // lod.players.inplay.s1
            _lmconfig.lod.lodPlayersInplayS1 = *(float*)pValue;
            LOG(L"lod.players.inplay.s1 = %03f", 
                    _lmconfig.lod.lodPlayersInplayS1);
            break;
        case 12: // lod.players.inplay.s2
            _lmconfig.lod.lodPlayersInplayS2 = *(float*)pValue;
            LOG(L"lod.players.inplay.s2 = %03f", 
                    _lmconfig.lod.lodPlayersInplayS2);
            break;
        case 13: // lod.players.inplay.s3
            _lmconfig.lod.lodPlayersInplayS3 = *(float*)pValue;
            LOG(L"lod.players.inplay.s3 = %03f", 
                    _lmconfig.lod.lodPlayersInplayS3);
            break;

        case 14: // lod.players.misc.s1
            _lmconfig.lod.lodPlayersMiscS1 = *(float*)pValue;
            LOG(L"lod.players.misc.s1 = %03f", 
                    _lmconfig.lod.lodPlayersMiscS1);
            break;
        case 15: // lod.players.misc.s2
            _lmconfig.lod.lodPlayersMiscS2 = *(float*)pValue;
            LOG(L"lod.players.misc.s2 = %03f", 
                    _lmconfig.lod.lodPlayersMiscS2);
            break;
        case 16: // lod.players.replay.s1
            _lmconfig.lod.lodPlayersReplayS1 = *(float*)pValue;
            LOG(L"lod.players.replay.s1 = %03f", 
                    _lmconfig.lod.lodPlayersReplayS1);
            break;
        case 17: // lod.players.other.s2
            _lmconfig.lod.lodPlayersReplayS2 = *(float*)pValue;
            LOG(L"lod.players.replay.s2 = %03f", 
                    _lmconfig.lod.lodPlayersReplayS2);
            break;
        case 18: // lod.players.replay.s3
            _lmconfig.lod.lodPlayersReplayS3 = *(float*)pValue;
            LOG(L"lod.players.replay.s3 = %03f", 
                    _lmconfig.lod.lodPlayersReplayS3);
            break;

        case 19: // lod.ref.inplay
            _lmconfig.lod.lodRefInplay = *(float*)pValue;
            LOG(L"lod.ref.inplay = %03f", 
                    _lmconfig.lod.lodRefInplay);
            break;
        case 20: // lod.ref.other
            _lmconfig.lod.lodRefReplay = *(float*)pValue;
            LOG(L"lod.ref.replay = %03f", 
                    _lmconfig.lod.lodRefReplay);
            break;
        case 21: // lod.players.entrance.s3
            _lmconfig.lod.lodPlayersEntranceS3 = *(float*)pValue;
            LOG(L"lod.players.entrance.s3 = %03f", 
                    _lmconfig.lod.lodPlayersEntranceS3);
            break;
        case 22: // lod.players.misc.s3
            _lmconfig.lod.lodPlayersMiscS3 = *(float*)pValue;
            LOG(L"lod.players.misc.s3 = %03f", 
                    _lmconfig.lod.lodPlayersMiscS3);
            break;
        case 23: // video.check.skipped
            pq = *(int*)pValue;
            if (pq == 0 || pq == 1 || pq == 2) {
                _lmconfig.pictureQuality = pq;
            }
            LOG(L"picture.quality = %d", 
                    _lmconfig.pictureQuality);
            break;
        case 24: // lod.active.player.ck.s1
            _lmconfig.lod.lodActivePlayerCKs1 = *(float*)pValue;
            LOG(L"lod.active.player.ck.s1 = %03f", 
                    _lmconfig.lod.lodActivePlayerCKs1);
            break;
        case 25: // lod.active.player.ck.s2
            _lmconfig.lod.lodActivePlayerCKs2 = *(float*)pValue;
            LOG(L"lod.active.player.ck.s2 = %03f", 
                    _lmconfig.lod.lodActivePlayerCKs2);
            break;
        case 26: // lod.active.player.ck.s3
            _lmconfig.lod.lodActivePlayerCKs3 = *(float*)pValue;
            LOG(L"lod.active.player.ck.s3 = %03f", 
                    _lmconfig.lod.lodActivePlayerCKs3);
            break;
        case 27: // lod.active.player.fk.s1
            _lmconfig.lod.lodActivePlayerFKs1 = *(float*)pValue;
            LOG(L"lod.active.player.fk.s1 = %03f", 
                    _lmconfig.lod.lodActivePlayerFKs1);
            break;
        case 28: // lod.active.player.fk.s2
            _lmconfig.lod.lodActivePlayerFKs2 = *(float*)pValue;
            LOG(L"lod.active.player.fk.s2 = %03f", 
                    _lmconfig.lod.lodActivePlayerFKs2);
            break;
        case 29: // lod.active.player.fk.s3
            _lmconfig.lod.lodActivePlayerFKs3 = *(float*)pValue;
            LOG(L"lod.active.player.fk.s3 = %03f", 
                    _lmconfig.lod.lodActivePlayerFKs3);
            break;
	}
}

BYTE getLB() 
{
    DWORD addr = dta[SCREEN_WIDTH];
    if (addr) {
        return *(BYTE*)(addr-1);
    }
    return 0;
}

void getResolution(DWORD& width, DWORD& height)
{
	DWORD protection;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;

    // get resolution
    if (VirtualProtect((BYTE*)dta[SCREEN_WIDTH], 4, newProtection, &protection))
        width = *(DWORD*)dta[SCREEN_WIDTH]; 
    if (VirtualProtect((BYTE*)dta[SCREEN_HEIGHT], 4, newProtection, &protection))
        height = *(DWORD*)dta[SCREEN_HEIGHT];
}

void setResolution(DWORD width, DWORD height)
{
	DWORD protection;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;

    // set resolution
    if (width>0 && height>0)
    {
        if (VirtualProtect((BYTE*)dta[SCREEN_WIDTH], 4, newProtection, &protection))
            *(DWORD*)dta[SCREEN_WIDTH] = width;
        if (VirtualProtect((BYTE*)dta[SCREEN_HEIGHT], 4, newProtection, &protection))
            *(DWORD*)dta[SCREEN_HEIGHT] = height;

        LOG(L"Resolution set: %dx%d", width, height);
    }
}

void setAspectRatio(float aspectRatio, bool manual)
{
	DWORD protection;
    DWORD newProtection = PAGE_EXECUTE_READWRITE;

    if (aspectRatio <= FLOAT_ZERO) // safety-check
        return;

    if (fabs(aspectRatio - 1.33333) < fabs(aspectRatio - 1.77777)) {
        // closer to 4:3
        *(DWORD*)dta[WIDESCREEN_FLAG] = 0;
        if (VirtualProtect(
                (BYTE*)dta[RATIO_4on3], 4, newProtection, &protection)) {
            *(float*)dta[RATIO_4on3] = aspectRatio;
        }
        LOG(L"Widescreen mode: no");
    } 
    else {
        // closer to 16:9
        *(DWORD*)dta[WIDESCREEN_FLAG] = 1;
        if (VirtualProtect(
                (BYTE*)dta[RATIO_16on9], 4, newProtection, &protection)) {
            *(float*)dta[RATIO_16on9] = aspectRatio;
        }
        LOG(L"Widescreen mode: yes");
    }

    /*
    if (VirtualProtect(
            (BYTE*)data[RATIO_4on3], 4, newProtection, &protection)) {
        *(float*)data[RATIO_4on3] = aspectRatio;
    }
    if (VirtualProtect(
            (BYTE*)data[RATIO_16on9], 4, newProtection, &protection)) {
        *(float*)data[RATIO_16on9] = aspectRatio;
    }
    */

    LOG(L"Aspect ratio: %0.5f", aspectRatio);
    LOG(L"Aspect ratio type: %s", (manual)?L"manual":L"automatic");
}

void lodAtModeCheckCallPoint()
{
    __asm {
        pushfd 
        push ebp
        push eax
        push ebx
        push edx
        push esi
        push edi
        mov [edi+0x13c], eax  // execute replaced code
        push ecx
        call lodAtModeCheck
        mov ecx,eax
        add esp,4     // pop parameters
        pop edi
        pop esi
        pop edx
        pop ebx
        pop eax
        pop ebp
        popfd
        retn
    }
}

void lodAtModeCheckCallPoint2010()
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
        call lodAtModeCheck
        mov edi,eax
        add esp,4     // pop parameters
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop ebp
        popfd
        mov dword ptr ds:[esi+0x13c],ecx // execute replaced code
        retn
    }
}

KEXPORT DWORD lodAtModeCheck(DWORD mode)
{
    if (mode != 0)
        return 1;
    return 0;
}

void lodAtSettingsReadPoint()
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
        call modifySettings
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

void lodAtSettingsResetPoint()
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
        call modifySettings
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

