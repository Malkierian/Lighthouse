#pragma once

// Dense handle for every declared setting, generated from defs/All.inc.
// C-safe: this is what the C shim in Settings_c.h takes.

#include "port/UI/cvar_prefixes.h"

typedef enum SettingId {
#define SETTING_INT(id, name, def, section) SETTING_##id,
#define SETTING_FLOAT(id, name, def, section) SETTING_##id,
#define SETTING_STRING(id, name, def, section) SETTING_##id,
#define SETTING_COLOR(id, name, r, g, b, a, section) SETTING_##id,
#define SETTING_COLOR24(id, name, r, g, b, section) SETTING_##id,
#include "port/Settings/defs/All.inc"
    SETTING_COUNT
} SettingId;
