#define UNICODE

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "utf8.h"

#include <map>
#include <hash_map>
#include <list>
#include <string>

template <typename T>
bool readMap(const wchar_t* cfgFile, T& m)
{
	FILE* f = _wfopen(cfgFile, L"rb");
	if (!f) return false;

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
	
	char currSection[64] = {'\0'};
	
	char a_str[BUFLEN];
	wchar_t str[WBUFLEN];

	wchar_t *pComment = NULL, *pName = NULL, *pValue = NULL, *pSpace = NULL, *pEq = NULL, *pTemp = NULL;

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

        if (wcslen(str)==0)
            continue;
		
		if (str[0] == '[') {
			wchar_t* pFirst = str + 1;
			wchar_t* pSecond = wcsstr(pFirst, L"]");
			if (!pSecond) continue;
			*pSecond = '\0';
			
			Utf8::fUnicodeToAnsi(currSection, pFirst);
			continue;
		}
		
		// skip comments
		pComment = wcsstr(str, L"#");
		if (pComment != NULL) pComment[0] = '\0';
			
		// parse the line
		pName = pValue = NULL;
		pEq = wcsstr(str, L",");
		if (pEq == NULL || pEq[1] == '\0') continue;

		pEq[0] = '\0';
		pName = str;
		pValue = pEq + 1;
		
        T::key_type numKey;
        int n = swscanf(pName, L"0x%x", &numKey);
        if (n!=1) n = swscanf(pName, L"%d", &numKey);
        if (n==1)
        {
            while (*pValue == ' ')
                pValue++;
                
            pTemp = pValue + wcslen(pValue) - 1;
            while (*pTemp == (wchar_t)13 || *pTemp == (wchar_t)10) {
                *pTemp = 0;
                pTemp--;
            }

            wstring sVal = pValue;
            m.insert(pair<T::key_type,wstring>(numKey,sVal));
        }
	}
	
    fclose(f);
	return true;
}
