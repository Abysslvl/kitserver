// afs2fs.h

#include <kconf.h>

#define MODID 100
#ifdef DEBUG
#define NAMELONG L"AFS2FS Module " KITSERVER_VERSION_STRING " (DEBUG)"
#else
#define NAMELONG L"AFS2FS Module " KITSERVER_VERSION_STRING
#endif
#define NAMESHORT L"AFS2FS"
#define DEFAULT_DEBUG 0

