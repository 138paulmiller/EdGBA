#ifndef DEFINES_H
#define DEFINES_H

#include <gba/gba.h>

#define EDGBA_APP_NAME "EdGBA"
#define EDGBA_ORG_NAME "138"
#define EDGBA_TITLE "EdGBA"
#define EDGBA_FILE_EXT "edgba"

// edgba editor config
#define CONFIG_KEY_GBA_EMU_PATH "gba_emulator_path"
#define CONFIG_KEY_RECENT_PROJECT "recent_project"
#define CONFIG_KEY_DEVKITPRO_PATH "devkitpro_path"
#define CONFIG_KEY_DARK_MODE "dark_mode"

// path relative to the edgba editor exe. Copied from external dir
#define EDITOR_DEVKITPRO_PATH "devkitPro"
#define EDITOR_LIBGBAK_PATH "libgbak"

#define EDITOR_LIBGBAK_RECOMPILE true

#define LOG_VERBOSE true

#endif // DEFINES_H
