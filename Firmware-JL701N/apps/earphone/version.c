#include "system/includes.h"
#include "generic/log.h"

extern char __VERSION_BEGIN[];
extern char __VERSION_END[];

_INLINE_
int app_version_check()
{
    /*lib_version_check();*/

    char *version;

    puts("=================Version===============\n");
    for (version = __VERSION_BEGIN; version < __VERSION_END;) {
        version += 4;
        printf("%s\n", version);
        version += strlen(version) + 1;
    }
    puts("=======================================\n");

    /*#ifdef CONFIG_FATFS_ENBALE
        VERSION_CHECK(fatfs, FATFS_VERSION);
    #endif

    #ifdef SDFILE_VERSION
        VERSION_CHECK(sdfile, SDFILE_VERSION);
    #endif*/
    return 0;
}


