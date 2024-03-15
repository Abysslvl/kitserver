#include <windows.h>
#include <stdio.h>

#include "afsreader.h"

#define MIN_ERROR_CODE -3
#define MAX_ERROR_CODE -1

// textual error messages
static char* unknownError = "UNKNOWN ERROR.";
static char* errorText[] = {
	"Cannot open AFS file.",
	"Heap allocation failed.",
	"Item not found in AFS.",
};

// Internal function prototypes
static DWORD FindItemInfoIndex(FILE* f, char* uniFileName, DWORD* pBase, DWORD* pIndex);

// Returns textual error message for a give code
char* GetAfsErrorText(DWORD code)
{
	if (code < MIN_ERROR_CODE || code > MAX_ERROR_CODE)
		return unknownError;

	int idx = -(int)code - 1;
	return errorText[idx];
}	

// Returns information about location of a file in AFS,
// given its name.
DWORD GetItemInfo(char* afsFileName, char* uniFileName, AFSITEMINFO* itemInfo, DWORD* pBase)
{
	FILE* f = fopen(afsFileName, "rb");
	if (f == NULL)
		return AFS_FOPEN_FAILED;

	DWORD index = 0;
	DWORD result = FindItemInfoIndex(f, uniFileName, pBase, &index);
	if (result != AFS_OK)
		return result;

	// fill-in the itemInfo
	fseek(f, *pBase + sizeof(AFSDIRHEADER) + sizeof(AFSITEMINFO) * index, SEEK_SET);
	fread(itemInfo, sizeof(AFSITEMINFO), 1, f);
	fclose(f);

	return result;
}

// Returns information about location of a file in AFS,
// given its id.
DWORD GetItemInfoById(char* afsFileName, int id, AFSITEMINFO* itemInfo, DWORD* pBase)
{
	FILE* f = fopen(afsFileName, "rb");
	if (f == NULL)
		return AFS_FOPEN_FAILED;

	DWORD index = id;
	*pBase = 0;

	// fill-in the itemInfo
	fseek(f, *pBase + sizeof(AFSDIRHEADER) + sizeof(AFSITEMINFO) * index, SEEK_SET);
	fread(itemInfo, sizeof(AFSITEMINFO), 1, f);
	fclose(f);

	return AFS_OK;
}

// Reads information about location of a file in AFS,
// given its id.
DWORD ReadItemInfoById(FILE* f, DWORD index, AFSITEMINFO* itemInfo, DWORD base)
{
	// fill-in the itemInfo
	fseek(f, base + sizeof(AFSDIRHEADER) + sizeof(AFSITEMINFO) * index, SEEK_SET);
	fread(itemInfo, sizeof(AFSITEMINFO), 1, f);
	return AFS_OK;
}

// Reads information about location of a file in AFS,
// given its id.
DWORD ReadItemInfoById(HANDLE hfile, DWORD index, AFSITEMINFO* itemInfo, DWORD base)
{
    DWORD bytesRead;

	// fill-in the itemInfo
	SetFilePointer(hfile, base + sizeof(AFSDIRHEADER) + sizeof(AFSITEMINFO) * index, NULL, FILE_BEGIN);
	ReadFile(hfile, itemInfo, sizeof(AFSITEMINFO), &bytesRead, 0);
	return AFS_OK;
}

// Returns information about where the corresponding AFSITEMINFO structure
// is located in the afs file. The answer consists of two parts: base, index.
// base   - is an offset to the beginning of AFS-folder that contains that AFSITEMINFO,
// index  - is the 0-based index of the AFSITEMINFO in the AFS-folder contents list.
static DWORD FindItemInfoIndex(FILE* f, char* uniFileName, DWORD* pBase, DWORD* pIndex)
{
	// remember current position
	DWORD afsBase = ftell(f);

	// read the header
	AFSDIRHEADER afh;
	fread(&afh, sizeof(AFSDIRHEADER), 1, f); 

	// skip the table of contents
	fseek(f, sizeof(AFSITEMINFO) * afh.dwNumFiles, SEEK_CUR);
	DWORD nameTableOffset = 0;
	fread(&nameTableOffset, sizeof(DWORD), 1, f);

	// seek the name-table
	fseek(f, afsBase + nameTableOffset, SEEK_SET);

	// allocate memory for name table read
	int nameTableSize = sizeof(AFSNAMEINFO) * afh.dwNumFiles;
	AFSNAMEINFO* fNames = (AFSNAMEINFO*)HeapAlloc(GetProcessHeap(), 
			HEAP_ZERO_MEMORY, nameTableSize);
	if (fNames == NULL)
		return AFS_HEAPALLOC_FAILED;

	fread(fNames, nameTableSize, 1, f); 
	
	// search for the filename
	int index = -1;
	for (int i=0; i<afh.dwNumFiles; i++)
	{
		if (strncmp(fNames[i].szFileName, uniFileName, 32)==0)
		{
			index = i;
			break;
		}
	}

	// free name-table memory
	HeapFree(GetProcessHeap(), 0, fNames);

	// check if we failed to find the file
	if (index == -1)
	{
		// File with given name was not found in this folder.
		// Recursively, navigate the child folders (if any exist) to
		// look for the file.
		fseek(f, afsBase + sizeof(AFSDIRHEADER), SEEK_SET);
		AFSITEMINFO* info = (AFSITEMINFO*)HeapAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, sizeof(AFSITEMINFO) * afh.dwNumFiles);
		if (info == NULL)
			return AFS_HEAPALLOC_FAILED;

		// read item info table
		fread(info, sizeof(AFSITEMINFO) * afh.dwNumFiles, 1, f);

		DWORD result = AFS_ITEM_NOT_FOUND;
		for (int k=0; k<afh.dwNumFiles; k++)
		{
			fseek(f, afsBase + info[k].dwOffset, SEEK_SET);

			AFSDIRHEADER fh;
			ZeroMemory(&fh, sizeof(AFSDIRHEADER));
			fread(&fh, sizeof(AFSDIRHEADER), 1, f);
			fseek(f, -sizeof(AFSDIRHEADER), SEEK_CUR);

			// check if this file is a child folder.
			if (fh.dwSig == AFSSIG)
			{
				result = FindItemInfoIndex(f, uniFileName, pBase, pIndex);
				if (result == AFS_OK) break;
			}
		}

		return result;
	}

	// set the base and index information.
	// The absolute offset of AFSITEMINFO structure in the file
	// can be calculated using this formula:
	//
	// absOffset = base + sizeof(AFSDIRHEADER) + sizeof(AFSITEMINFO) * index 
	//
	*pBase = afsBase;
	*pIndex = index;

	return AFS_OK;
}

