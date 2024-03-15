#define UNICODE

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <hash_map>

#include "utf8.h"
#include "lang.h"


hash_map<string, wstring> g_transl;
hash_map<string, wstring>::iterator g_translIt;
const wchar_t noTransl[] = L"(Translation missing)";

void clearTransl() {
	g_transl.clear();
}

const wchar_t* _getTransl(char* section, char* key) {
	char buf[BUFLEN];
	ZeroMemory(buf, BUFLEN);
	sprintf(buf, "[%s]%s", section, key);
	string sKey = buf;

	if (!MAP_CONTAINS(g_transl, sKey)) return noTransl;
	return g_transl[sKey].c_str();
}

void readLang(wchar_t* langName, HMODULE hMod) {
	wchar_t langFile[BUFLEN];
	ZeroMemory(langFile, WBUFLEN);
	swprintf(langFile, L".\\lang_%s.txt", langName);
	readLangFile(langFile, hMod);
	return;
}

void readLangFile(wchar_t* langFile, HMODULE hMod) {
	bool isTempFile = false;
	char tmpName[BUFLEN];
	char* tmpName2 = tmpName + 1;
	ZeroMemory(tmpName, BUFLEN);
	
	clearTransl();

	FILE* f = _wfopen(langFile, L"rb");
	
	if (!f) {
		// load the translation from the resource to a temporary file
		HRSRC rsrc = 	FindResource(hMod, L"#1001", L"BINARY");
		BYTE* res = (BYTE*)LoadResource(hMod, rsrc);
		BYTE* endRes = res;
		while (*endRes != 0) endRes++;
		
		tmpnam(tmpName);
		FILE * pFile = fopen(tmpName2, "wb");
		fwrite(res, endRes - res, 1, pFile);
		fclose(pFile);
		
		isTempFile = true;
		f = fopen(tmpName2, "rb");
	}
		
	DWORD firstDWORD;
	bool unicodeFile = false;
	fgets((char*)&firstDWORD, 4, f);
	if ((firstDWORD & 0xffffff) == 0xbfbbef) {
		// UTF8
		fseek(f, 3, SEEK_SET);
	} else if ((firstDWORD & 0xffff) == 0xfeff) {
		// Unicode Little Endian
		unicodeFile = true;
		fseek(f, 2, SEEK_SET);
	} else {
		// no supported BOM detected, asume UTF8
		fseek(f, 0, SEEK_SET);
	}
	
	wchar_t currSection[256] = {'\0'};

	char a_str[BUFLEN];
	wchar_t str[BUFLEN];
	wchar_t buf[BUFLEN];
	wchar_t val[BUFLEN];
	wchar_t *pFirst = NULL, *pSecond = NULL, *pSpace = NULL, *pEq = NULL;
	while (!feof(f))
	{
		if (!unicodeFile) {
			ZeroMemory(str, WBUFLEN);
			ZeroMemory(a_str, BUFLEN);
			fgets(a_str, BUFLEN-1, f);
			Utf8::fUtf8ToUnicode(str, a_str);
		} else {
			ZeroMemory(str, WBUFLEN);
			fgetws(str, BUFLEN-1, f);
		}
		ZeroMemory(val, WBUFLEN);
		
		pFirst = pSecond = NULL;
		
		if (str[0] == '[') {
			pFirst = str + 1;
			pSecond = wcsstr(pFirst, L"]");
			if (!pSecond) continue;
			*pSecond = '\0';
			
			wcscpy(currSection, pFirst);												
			continue;
		}
		
		pFirst = wcsstr(str, L"$$");
		if (!pFirst) continue;
		pFirst += 2;

		pSpace = wcsstr(str, L" ");
		pEq = wcsstr(str, L" ");
		if (!pSpace || pEq < pSpace)  pSpace = pEq;
		if (pSpace > pFirst)
			continue;
		else
			*pSpace = '\0';
			
		ZeroMemory(buf, WBUFLEN);
		swprintf(buf, L"[%s]%s", currSection, str);
		char* buf2 = Utf8::unicodeToAnsi(buf);
		string sKey = buf2;
		Utf8::free(buf2);
			
		pSecond = wcsstr(pFirst, L"$$");
		while (!pSecond && !feof(f))
		{
			wcscat(val, pFirst);
			
			if (!unicodeFile) {
				ZeroMemory(str, WBUFLEN);
				ZeroMemory(a_str, BUFLEN);
				fgets(a_str, BUFLEN-1, f);
				Utf8::fUtf8ToUnicode(str, a_str);
			} else {
				ZeroMemory(str, WBUFLEN);
				fgetws(str, BUFLEN-1, f);
			}
			
			pFirst = str;
			pSecond = wcsstr(pFirst, L"$$");
		}
		*pSecond = '\0';
		wcscat(val, pFirst);
		
		wstring sVal = val;
		g_transl[sKey] = sVal;
	}
	fclose(f);
	
	if (isTempFile)
		DeleteFileA(tmpName2);
	
	return;
}