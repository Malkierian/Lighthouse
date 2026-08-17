#pragma once

// Minimal C-callable view of the settings table, for decomp/port code that cannot
// include the C++ header. C++ callers should use the typed objects in Settings.h.
// A type mismatch between the id and the accessor is a no-op read/write, not UB.

#include <stdint.h>
#include <stdbool.h>

#include "port/Settings/SettingIds.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t Setting_GetInt(SettingId id);
float Setting_GetFloat(SettingId id);
const char* Setting_GetString(SettingId id);

void Setting_SetInt(SettingId id, int32_t value);
void Setting_SetFloat(SettingId id, float value);
void Setting_SetString(SettingId id, const char* value);

bool Setting_IsDefault(SettingId id);
bool Setting_IsExplicitlySet(SettingId id);
void Setting_Reset(SettingId id);

#ifdef __cplusplus
}
#endif
