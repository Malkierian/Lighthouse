#pragma once

// One object per declared setting, generated from defs/All.inc. These are the
// intended read/write API:
//
//   int32_t v = Settings::RANDO_NOTIFICATIONS;     // implicit conversion
//   Settings::RANDO_NOTIFICATIONS = 0;             // writes through to the CVar store
//   if (!Settings::RANDO_NOTIFICATIONS.IsDefault()) { ... }
//
// Reads hit the cached value, not the CVar hashmap.

#include "port/Settings/Setting.h"
// Table-dependent by definition: supplies the SETTING_* enumerators and, via
// cvar_prefixes.h, the CVAR_* name macros the fragments expand to.
#include "port/Settings/SettingIds.h"

namespace Settings {

#define SETTING_INT(id, name, def, section) inline constinit Int id{ SETTING_##id, name, def, section };
#define SETTING_FLOAT(id, name, def, section) inline constinit Float id{ SETTING_##id, name, def, section };
#define SETTING_STRING(id, name, def, section) inline constinit String id{ SETTING_##id, name, def, section };
#define SETTING_COLOR(id, name, r, g, b, a, section) \
    inline constinit Color id{ SETTING_##id, name, Color_RGBA8{ r, g, b, a }, section };
#define SETTING_COLOR24(id, name, r, g, b, section) \
    inline constinit Color24 id{ SETTING_##id, name, Color_RGB8{ r, g, b }, section };
#include "port/Settings/defs/All.inc"

// Every setting in declaration order, for enumeration (preset apply, reset-all, the C shim).
// Generated rather than self-registered: registering into a container during static
// init is what actually causes initialisation-order problems.
extern Base* const gAll[SETTING_COUNT];

// Null for an out-of-range id.
Base* Get(SettingId id);

// Populate every cached value from the CVar store. Call once at startup, after the
// LUS context has loaded CVars and before anything reads a setting. Deliberately does
// not fire OnChange — the subsystems a callback would poke do not exist yet.
void Init();

// Re-sync every cached value, firing OnChange for each one that moved. Needed after
// anything that writes the CVar store behind the cache's back — preset application,
// CVarClearBlock, the console.
void RefreshAll();

} // namespace Settings
