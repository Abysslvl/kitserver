#define UNICODE
#define WINDOW_TITLE L"Kitserver 12"
#define KEXPORT EXTERN_C __declspec(dllexport)

#include <map>

using namespace std;

#define MAP_FIND(map,key) (((*(map)).find(key) != (*(map)).end()) ? (*(map))[key] : NULL)
#define MAP_CONTAINS(map,key) (map.find(key)!=map.end())
#define VECTOR_FOREACH(vector,it) for (it = (vector).begin(); it < (vector).end();  it++)
#define CALLCHAIN(hookid,it) for (vector<void*>::iterator it = g_callChains[(hookid)].begin(); it < g_callChains[(hookid)].end();  it++)
// safe iterator: allows for modifications of g_callChains[hookId] vector during the iteration
#define CALLCHAIN_BEGIN(hookid,it) {\
    vector<void*> v;\
    CALLCHAIN(hookid, vit) v.push_back(*vit);\
    for (vector<void*>::iterator it = v.begin(); it != v.end(); it++)
#define CALLCHAIN_END }

#define BUFLEN 4096
#define SW sizeof(wchar_t)
#define WBUFLEN (SW * BUFLEN)
#define NULLSTRING L"\0"
