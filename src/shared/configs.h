#include <string>

using namespace std;

#define MAP_FIND(map,key) (((*(map)).find(key) != (*(map)).end()) ? (*(map))[key] : NULL)
#define MAP_CONTAINS(map,key) (map.find(key)!=map.end())

#define BUFLEN 4096
#define SW sizeof(wchar_t)
#define WBUFLEN (SW * BUFLEN)

// pValue is a pointer to a variable of the type specified in the config handler
// "a" might contain some info for the function, ''
typedef void  (*PROCESSCONFIG)(char* pName, const void* pValue, DWORD a);

#ifndef _KLOAD_H
enum {DT_NORMAL, DT_STRING, DT_DWORD, DT_INT, DT_DOUBLE, DT_FLOAT};
#define C_ALL 0x80000000
#endif

bool readConfig(const wchar_t* cfgFile);
bool writeConfig(const wchar_t* cfgFile);
void _getConfig(char* section, char* name, BYTE dataType, DWORD a, PROCESSCONFIG callback);
void _setConfig(char* section, char* name, wstring& value, bool replace=true);
void _removeConfig(char* section, char* name);

// for reading map.txt files
template <typename T>
bool readMap(const wchar_t* cfgFile, T& m);
