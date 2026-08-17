// Master settings list. Included multiple times with different macro definitions
// (see SettingIds.h, Settings.h, Settings.cpp), so it deliberately has no include
// guard. Define only the macros you care about before including this; the rest
// expand to nothing and all of them are undefined again on the way out.
//
//   SETTING_INT     (id, name, default, presetSection)
//   SETTING_FLOAT   (id, name, default, presetSection)
//   SETTING_STRING  (id, name, default, presetSection)
//   SETTING_COLOR   (id, name, r, g, b, a, presetSection)
//   SETTING_COLOR24 (id, name, r, g, b, presetSection)
//
// `id` becomes both the enumerator SETTING_<id> and the object Settings::<id>.
// `default` must be a literal usable in a constant expression from Settings.h;
// anything needing a header (enum constants, helper functions) has to have that
// header reachable there, so prefer plain literals.
// Use PRESET_SECTION_MAX for settings that belong to no preset block.

#ifndef SETTING_INT
#define SETTING_INT(id, name, def, section)
#endif
#ifndef SETTING_FLOAT
#define SETTING_FLOAT(id, name, def, section)
#endif
#ifndef SETTING_STRING
#define SETTING_STRING(id, name, def, section)
#endif
#ifndef SETTING_COLOR
#define SETTING_COLOR(id, name, r, g, b, a, section)
#endif
#ifndef SETTING_COLOR24
#define SETTING_COLOR24(id, name, r, g, b, section)
#endif

#include "Settings.inc"
#include "Rando.inc"
#include "Network.inc"

#undef SETTING_INT
#undef SETTING_FLOAT
#undef SETTING_STRING
#undef SETTING_COLOR
#undef SETTING_COLOR24
