#define UNICODE

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "utf8.h"
#include "configs.h"
#include "configs.hpp"

#include <map>
#include <hash_map>
#include <list>
#include <string>

multimap<string, wstring> g_knownConfigs;
multimap<string, wstring>::iterator g_knownConfigsIt;

bool readConfig(const wchar_t* cfgFile)
{
	FILE* f = _wfopen(cfgFile, L"rb");
	if (!f) return false;

    // clear the map
    g_knownConfigs.clear();

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
	char name[BUFLEN];
	wchar_t buf[WBUFLEN];

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
		pEq = wcsstr(str, L"=");
		if (pEq == NULL || pEq[1] == '\0') continue;

		pEq[0] = '\0';
		pName = str;
		pValue = pEq + 1;
		
		ZeroMemory(name, BUFLEN);
		ZeroMemory(buf, WBUFLEN);
		swscanf(pName, L"%s", buf);
		Utf8::fUnicodeToAnsi(name, buf);
		
		char buf2[BUFLEN];
		ZeroMemory(buf2, BUFLEN);
		sprintf(buf2, "[%s]%s", currSection, name);
		string sKey = buf2;
		
		while (*pValue == ' ')
			pValue++;
			
		pTemp = pValue + wcslen(pValue) - 1;
		while (*pTemp == (wchar_t)13 || *pTemp == (wchar_t)10) {
			*pTemp = 0;
			pTemp--;
		}

		wstring sVal = pValue;

		g_knownConfigs.insert(pair<string,wstring>(sKey,sVal));
	}
	
    fclose(f);
	return true;
}

bool writeConfig(const wchar_t* cfgFile)
{
	FILE* f = _wfopen(cfgFile, L"wb");
	if (!f) return false;

    // create list of section-keys
    map<string,bool> sectionKeys;
    for (multimap<string, wstring>::iterator cit = g_knownConfigs.begin();
            cit != g_knownConfigs.end(); cit++)
    {
        // split the key
        int p1 = cit->first.find('[');
        int p2 = cit->first.find(']');
        string section(cit->first.substr(p1+1,p2-p1-1));
        sectionKeys.insert(pair<string,bool>(section,true));
    }

    // write to file
	DWORD firstDWORD = 0xbfbbef; // Utf-8
	fputs((char*)&firstDWORD, f);

	char a_str[BUFLEN];
	wchar_t str[WBUFLEN];
	
    for (map<string,bool>::iterator it = sectionKeys.begin(); 
            it != sectionKeys.end(); it++)
    {
        fprintf(f,"[%s]\r\n",it->first.c_str());
        for (multimap<string, wstring>::iterator vit = g_knownConfigs.begin(); 
                vit != g_knownConfigs.end(); vit++)
        {
            int p1 = vit->first.find('[');
            int p2 = vit->first.find(']');
            string section(vit->first.substr(p1+1,p2-p1-1));
            string key(vit->first.substr(vit->first.find(']')+1));
            if (section != it->first)
               continue;

            fprintf(f,"%s = ",key.c_str());

			ZeroMemory(str, WBUFLEN);
			ZeroMemory(a_str, BUFLEN);
            swprintf(str,L"%s",vit->second.c_str());
			Utf8::fUnicodeToUtf8((unsigned char*)a_str, str);
			fputs(a_str, f);

            fprintf(f,"\r\n");
        }
        fprintf(f,"\r\n");
	}

    fclose(f);
	return true;
}

void _getConfig(char* section, char* name, BYTE dataType, DWORD a, PROCESSCONFIG callback) {

	if (!section || !name || !callback) return;
		
	wchar_t *startQuote, *endQuote;
	char buf[BUFLEN];
	wchar_t buf2[WBUFLEN];
	DWORD dValue;
	int iValue;
	double dbValue;
	float fValue;
		
	ZeroMemory(buf, BUFLEN);
	sprintf(buf, "[%s]%s", section, name);
	string sKey = buf;	
	
	pair<multimap<string, wstring>::iterator,multimap<string, wstring>::iterator> ret;
	ret = g_knownConfigs.equal_range(sKey);
	int i = g_knownConfigs.count(sKey);
	
	for (g_knownConfigsIt = ret.first; g_knownConfigsIt != ret.second; g_knownConfigsIt++, i--) {
		// the highest bit of a is an indicator whether to return all values for this setting or not
		if (((a & C_ALL) == 0)  && (i != 1)) continue;
		
		wchar_t* value = (wchar_t*)g_knownConfigsIt->second.c_str();
	
		switch (dataType) {
			case DT_STRING:
				startQuote = wcsstr(value, L"\"");
				if (startQuote == NULL) {
					callback(buf, value, a & 0x7fffffff);
					continue;
				}
				endQuote = wcsstr(startQuote + 1, L"\"");
				if (endQuote == NULL) {
					callback(buf, value, a & 0x7fffffff);
					continue;
				}

				ZeroMemory(buf2, WBUFLEN);
				memcpy(buf2, startQuote + 1, SW*(endQuote - startQuote - 1));
				
				callback(buf, buf2, a & 0x7fffffff);
				break;
				
			case DT_DWORD:
				if (swscanf(value, L"%d", &dValue) != 1) continue;
				callback(buf, &dValue, a & 0x7fffffff);
				break;
				
			case DT_INT:
				if (swscanf(value, L"%d", &iValue) != 1) continue;
				callback(buf, &iValue, a & 0x7fffffff);
				break;
				
			case DT_DOUBLE:
				if (swscanf(value, L"%f", &dbValue) != 1) continue;
				callback(buf, &dbValue, a & 0x7fffffff);
				break;

			case DT_FLOAT:
				if (swscanf(value, L"%f", &fValue) != 1) continue;
				callback(buf, &fValue, a & 0x7fffffff);
				break;
			
			case DT_NORMAL:
			default:
				callback(buf, value, a & 0x7fffffff);
		}
			/*	
				*/
	}
	return;
}

void _removeConfig(char* section, char* name)
{
    char buf[BUFLEN];
    sprintf(buf,"[%s]%s", section, name);
    string key(buf);

    pair<multimap<string, wstring>::iterator,multimap<string, wstring>::iterator> ret;
    ret = g_knownConfigs.equal_range(key);
    g_knownConfigs.erase(ret.first,ret.second);
}

void _setConfig(char* section, char* name, wstring& value, bool replace) 
{
    char buf[BUFLEN];
    sprintf(buf,"[%s]%s", section, name);
    string key(buf);

    if (replace) _removeConfig(section, name);
    g_knownConfigs.insert(pair<string,wstring>(key,value));
}

void unitTest()
{
    hash_map<WORD,wstring> m1;
    readMap(L"map.txt", m1);
}
