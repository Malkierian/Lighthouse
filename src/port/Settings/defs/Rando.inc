// gRandoSettings.* / gRando.* — randomizer settings. Included by All.inc; no include guard.

SETTING_INT   (RANDO_NOTIFICATIONS, CVAR_RANDOMIZER_SETTING("RandoNotifications"), 1,  PRESET_SECTION_RANDOMIZER)
SETTING_STRING(RANDO_INPUT_SEED,    CVAR_RANDOMIZER_SETTING("InputSeed"),          "", PRESET_SECTION_RANDOMIZER)

// The check tracker lives under gRando.*, which no preset block covers.
SETTING_COLOR (CHECK_TRACKER_LOGIC_COLOR, "gRando.CheckTracker.LogicColor.Value", 200, 200, 200, 255, PRESET_SECTION_MAX)
