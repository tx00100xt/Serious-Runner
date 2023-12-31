#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>

enum MESSAGETYPE {
    INFO,
    WARN,
    ERR
};

enum DOWNLOADTYPE {
    LEVEL,
    GAME,
    MOD,
    DB
};

enum DIFFICULTY {
    EASY,
    TOURIST,
    NORMAL,
    HARD,
    SERIOUS
};

enum FE_TABLES {
    FE_OFFICIAL,
    FE_BESTMAPS,
    FE_USERSMAPS,
    FE_MODS
};

enum SE_TABLES {
    SE_OFFICIAL,
    SE_BESTMAPS,
    SE_HQMAPS,
    SE_USERSMAPS,
    SE_MODS
};

enum BOOLTYPES {
    BOOLFALSE,
    BOOLTRUE
};

enum PROGRESSBARINDEX {
    INSTALL_PBAR,
    FE_BESTMAPS_PBAR,
    FE_USERSMAPS_PBAR,
    FE_MODS_PBAR,
    SE_BESTMAPS_PBAR,
    SE_HQMAPS_PBAR,
    SE_USERSMAPS_PBAR,
    SE_MODS_PBAR,
};

enum COPYPROGRESSINDEX {
    INSTALL_COPYPROGRESS,
    FE_MODS_COPYPROGRESS,
    SE_MODS_COPYPROGRESS,
};

enum UNPACKPROGRESSINDEX {
    INSTALL_UNPACKPROGRESS,
    FE_MODS_UNPACKPROGRESS,
    SE_MODS_UNPACKPROGRESS,
};

enum DISTROFLAG {
    FREEBSD,
    ALPINE,
    LINUX,
    OTHER,
    WINDOWS
};

enum APPTHEMES {
    DEFAULTTHEME,
    AMOLED,
    AQUA,
    CONSOLESTYLE,
    ELEGANTDARK,
    MACOSTHEME,
    MANJAROMIX,
    MATERIALDARKE,
    NEONBUTTONS,
    UBUNTUTHEME
};
#endif // TYPES_H
