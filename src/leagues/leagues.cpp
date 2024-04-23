#define UNICODE
#define THISMOD &kLeagues

#include <Windows.h>
#include <kload_exp.h>
#include "leagues.h"
#include "leagues_addr.h"
#include <kconf.h>
#include <dllinit.h>

KMOD kLeagues = { MODID, NAMELONG, NAMESHORT, DEFAULT_DEBUG };

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface);
static void blocksReadCallPoint();
void ReadBlocks(DWORD lastblock, DWORD numTeamsAddr, DWORD totalBlocks);
static void changueBlock();
static int ReadMap(const wchar_t* mapFile);
void clubsReadCallPoint();
DWORD clubsRead();
DWORD leaguesRead(League* theleagues, int firstSlot);
void changeRelink();
void changeLogo();
void changeFlag();

static LeagueBlock* blocks;
static DWORD aux1, aux2;
static League* clubs;
static DWORD codeMakeLeague, codeBeforeReadFlag, retReadBlocks;
static DWORD relinkOffset, logoOffset, texOffset, flagOffset;
static DWORD relink;
static char* thistex;
static DWORD customLeague;
static BYTE* flags;
static BYTE leagueFlag;

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        RegisterKModule(THISMOD);

        // check game and kload version
        if (!checkGameVersion())
        {
            LOG(L"Sorry, your game version isn't supported!");
            return false;
        }
        CHECK_KLOAD(MAKELONG(0, KITSERVER_VERSION_MAJOR));

        // copy addresses and init module
        copyAdresses();
        hookFunction(hk_D3D_CreateDevice, initModule);
        break;

    case DLL_PROCESS_DETACH:
        LOG(L"Cleaning memory ...");
        if (blocks != nullptr)
        {
            for (int i = 0; i < _msize(blocks)/sizeof(LeagueBlock); i++)
                delete[] blocks[i].block;
            delete[] blocks;
        }
        if (clubs != nullptr)
        {
            for (int i = 0; i < _msize(clubs) / sizeof(League); i++)
            {
                delete[] clubs[i].name;
                delete[] clubs[i].logo;
            }
            delete[] clubs;
        }
        if (flags != nullptr)
            delete[] flags;

        LOG(L"Shutting down this module ...");
        break;
    }

    return TRUE;
}

HRESULT STDMETHODCALLTYPE initModule(IDirect3D9* self, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    unhookFunction(hk_D3D_CreateDevice, initModule);
    LOG(L"Initializing Leagues Module");

    codeMakeLeague = code[C_MAKE_LEAGUE];
    codeBeforeReadFlag = code[C_BEFORE_READ_FLAG];
    retReadBlocks = code[C_READ_TOTAL_BLOCKS] + 5;

    relinkOffset = dta[LEAGUES_RELINK_TABLE];
    logoOffset = dta[LEAGUES_LOGO_TABLE];
    texOffset = dta[LEAGUES_TEX_TABLE];
    flagOffset = dta[LEAGUES_FLAG_TABLE];

    HookJmpPoint(code[C_READ_TOTAL_BLOCKS], blocksReadCallPoint, 4, 0);
    HookCallPoint(code[C_READ_BLOCK], changueBlock, 3, 1);
    HookCallPoint(code[C_READ_RELINK], changeRelink, 3, 2);
    HookCallPoint(code[C_READ_LOGO], changeLogo, 3, 9);
    HookCallPoint(code[C_READ_FLAG], changeFlag, 3, 27);

    wstring mapDir = getPesInfo()->gdbDir;
    mapDir += L"GDB\\leagues\\map.txt";
    int retval = ReadMap(mapDir.c_str());
    if (retval == 0)
    {
        LOG(L"Found club leagues: %d", _msize(clubs)/sizeof(League));
        HookCallPoint(code[C_READ_CLUBS], clubsReadCallPoint, 6, 2);
        LOG(L"Using league-server club leagues");
    }
    else
    {
        if (retval == -1)
            LOG(L"No map.txt file found.");
        else 
            LOG(L"No club leagues found");

        LOG(L"Using game-set club leagues");
    }

    LOG(L"Initialization complete.");
    return D3D_OK;
}

void clubsReadCallPoint()
{
    __asm {
        test bl, 01 // skip clubs in UCL mode
        jne skipClubs
        pushfd
        push eax
        push ebx
        push ecx
        push edx
        push edi
        push esi
        push ebp
        call clubsRead
        pop ebp
        pop esi
        pop edi
        pop edx
        pop ecx
        pop ebx
        pop eax
        popfd
        skipClubs :
        add esp, 4 // pop up return address
        pop esi
        pop ebx
        retn 4
    }
}

DWORD clubsRead()
{
    customLeague = 0;

    leaguesRead(clubs, 100);

    customLeague = 0xff;

    return 0;
}

DWORD leaguesRead(League* theleagues, int firstSlot)
{
    char* leaguename;
    for (int i = 0, slot = firstSlot; i < _msize(theleagues)/sizeof(League); i++, slot++)
    {
        relink = theleagues[i].relink;
        leaguename = theleagues[i].name;
        thistex = theleagues[i].logo;

        // assembly fragment to call the league reading function
        __asm {
            mov ecx, esi
            push leaguename
            push 0
            push 1
            push slot
            call codeMakeLeague
        }
    }
    return 0;
}

void changeRelink()
{
    if (customLeague != 0xff)
    {
        __asm {
            mov ecx, relink
            retn
        }
    }
    else
    {
        __asm {
            push eax
            mov eax, relinkOffset
            mov ecx, dword ptr ds : [esi * 4 + eax]
            pop eax
            retn
        }
    }
}

void changeLogo()
{
    if (customLeague != 0xff)
    {
        __asm {
            mov ecx, 0xFFFFFFFF
            mov edx, thistex
            retn
        }
    }
    else
    {
        __asm {
            push eax
            mov eax, logoOffset
            mov ecx, dword ptr ds : [esi * 4 + eax]
            mov eax, texOffset
            mov edx, dword ptr ds : [esi * 4 + eax]
            pop eax
            retn
        }
    }
}

void changeFlag()
{
    __asm {
        cmp eax, 0x64 // slot 100
        jae CustomLeague
        cmp eax, 0x1D
        jae BeforeReadFlag
        mov ecx, flagOffset
        cmp byte ptr[eax + ecx], -01
        je BeforeReadFlag
        mov ecx, esi
        mov eax, [ecx + 0x44]
        mov ecx, [esp + 0x1C]
        push eax
        mov eax, flagOffset
        movzx ecx, byte ptr[ecx + eax]
        pop eax
        retn
        CustomLeague:
        push edx
        mov aux1, eax // store the slot
    }
        leagueFlag = flags[aux1 - 100];
    __asm {
        mov eax, aux1
        pop edx
        cmp leagueFlag, -1
        je BeforeReadFlag
        mov ecx, esi
        mov eax, [ecx + 0x44]
        mov ecx, [esp + 0x1C]
        movzx ecx, leagueFlag
        retn
        BeforeReadFlag :
        add esp, 04
        jmp codeBeforeReadFlag
    }
}

static void blocksReadCallPoint()
{
    __asm {
        add eax, edx
        push ebx
        push eax
        push ebx
        cmp [esp+0x34], LEAGUES_LAST_BLOCK
        je ReadBlocks_
        jmp retReadBlocks // return
        ReadBlocks_:
        push ebx
        push edx // param: total blocks
        push ecx // param: number of teams addr
        push eax // param: last block addr
        call ReadBlocks
        pop eax
        pop ecx
        pop edx
        pop ebx
        jmp retReadBlocks // return
    }
}


static void changueBlock()
{
    __asm {
        mov aux1, eax
    }
    if (aux1 >= LEAGUES_NEW_BLOCKS_SLOT)
    {
        __asm 
        {
            push eax
            push edx
            push ecx
            add esp, 0xc
            push eax
        }
        // set the league teams number
        aux2 = blocks[aux1 - LEAGUES_NEW_BLOCKS_SLOT].numTeams;
        __asm
        {
            // apply teams number
            add esp, 0x18
            mov edx, aux2
            mov [esp], edx
            sub esp, 0x18
            pop eax
            mov aux1, eax
        }
        // set the league block address
        aux2 = (DWORD) blocks[aux1 - LEAGUES_NEW_BLOCKS_SLOT].block;
        __asm
        {
            // apply block address
            add esp, 0x18
            mov edx, aux2
            mov [esp], edx
            sub esp, 0x18
            sub esp, 0xc
            pop ecx
            pop edx
            pop eax
        }
    }
    __asm
    {
        // continue with regular instructions
        mov eax, [esp+0x14]
        cmp eax, edi
        ret
    }
}

void ReadBlocks(DWORD lastblock, DWORD numTeamsAddr, DWORD totalBlocks)
{
    // get the extra blocks num && create the blocks buffer
    int blocksCount = (*(DWORD*)totalBlocks) - LEAGUES_LAST_BLOCK - 1;
    blocks = new LeagueBlock[blocksCount];

    // get the number of teams of the last block
    numTeamsAddr += 4;
    int numTeams = (*(DWORD*)(numTeamsAddr)) / 2;

    DWORD nextblock = lastblock;

    int count = 0;
    // while next block exists && not over the limit
    for (int count = 0; count < blocksCount; count++)
    {
        // calculate the next block
        if (numTeams % 8 == 0)
            nextblock = lastblock + numTeams * 2;
        else
            nextblock = lastblock + (numTeams / 8 + 1) * 16;
        lastblock = nextblock;

        // calculate the next num of teams
        numTeamsAddr += 12;
        numTeams = (*(DWORD*)(numTeamsAddr)) / 2;

        LOG(L"Found league block %d", LEAGUES_NEW_BLOCKS_SLOT + count);

        // create the new league block
        blocks[count].numTeams = numTeams;
        blocks[count].block = new WORD[numTeams];
        TRACE(L"League block %d address: %x", LEAGUES_NEW_BLOCKS_SLOT + count, blocks[count].block);
        TRACE(L"League block %d num of teams: %d", LEAGUES_NEW_BLOCKS_SLOT + count, blocks[count].numTeams);

        // set the teams in the block
        for (int i = 0; i < numTeams; i++)
        {
            blocks[count].block[i] = *(WORD*)nextblock;
            nextblock += 2;
            TRACE(L"League block %d Team %d: %x", LEAGUES_NEW_BLOCKS_SLOT + count, i + 1, tempBlocks[count].block[i]);
        }
    }

    LOG(L"Total new league blocks readed: %d", blocksCount);
}

static int ReadMap(const wchar_t* mapFile)
{
    FILE* map = _wfopen(mapFile, L"r");
    if (map == nullptr)
        return -1;

    wchar_t line[MAX_PATH];
    League temp[LEAGUES_MAX_SLOT];
    BYTE flagsTemp[LEAGUES_MAX_SLOT];
    wchar_t leagueName[50];
    wchar_t leagueLogo[50];

    // read the map file
    int count = 0;
    while (count < LEAGUES_MAX_SLOT && fgetws(line, 100, map) != 0)
    {
        if (line[0] == L'#')
            continue;
        if (line[0] == L'\n')
            continue;

        if (swscanf(line, L"%d, \"%[^\"]\", \"%[^\"]\", %d", &temp[count].relink, leagueName, leagueLogo,
            &flagsTemp[count]) == 4)
        {
            char buf[50];
            // convert from utf8-wchar_t* to utf8-char*
            if (leagueLogo[wcslen(leagueLogo)] == '\0')
            {
                wcstombs(buf, leagueLogo, 50);
                temp[count].logo = new char[strlen(buf) + 1];
                strcpy(temp[count].logo, buf);
            }

            if (leagueName[wcslen(leagueName)] == '\0')
            {
                wcstombs(buf, leagueName, 50);
                temp[count].name = new char[strlen(buf) + 1];
                strcpy(temp[count].name, buf);
            }
            count++;
        }
    }
    fclose(map);

    if (count == 0)
        return -2;

    // create the real clubs && flags
    clubs = new League[count];
    flags = new BYTE[count];
    for (int i = 0; i < count; i++)
    {
        flags[i] = flagsTemp[i];
        clubs[i] = temp[i];
    }
    return 0;
}