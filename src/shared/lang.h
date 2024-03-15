
void readLang(wchar_t* langName, HMODULE hMod);
void readLangFile(wchar_t* langFile, HMODULE hMod);
const wchar_t* _getTransl(char* section, char* key);

using namespace std;

#define MAP_FIND(map,key) (((*(map)).find(key) != (*(map)).end()) ? (*(map))[key] : NULL)
#define MAP_CONTAINS(map,key) (map.find(key)!=map.end())

#define BUFLEN 4096
#define SW sizeof(wchar_t)
#define WBUFLEN (SW * BUFLEN)
