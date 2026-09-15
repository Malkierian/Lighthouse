#include "SettingsPrefs.h"
#include "port/UI/cvar_prefixes.h"

namespace Prefs::Settings {

namespace Windows {

static Prefs::Options<bool> Synced(const char* cvar) {
    return Prefs::Options<bool>().CVar(cvar).SyncCVar();
}

std::string confPath = "Windows.";
Prefs::Bool Menu{ PrefSection::SECTION_SETTINGS, confPath + "Menu", false, Synced(CVAR_WINDOW("Menu")) };
Prefs::Bool ControllerConfiguration{ PrefSection::SECTION_SETTINGS, confPath + "ControllerConfiguration", false,
                                     Synced(CVAR_WINDOW("ControllerConfiguration")) };
Prefs::Bool GamepadMapper{ PrefSection::SECTION_SETTINGS, confPath + "GamepadMapper", false,
                           Synced(CVAR_WINDOW("GamepadMapper")) };
Prefs::Bool ModMenu{ PrefSection::SECTION_SETTINGS, confPath + "ModMenu", false, Synced(CVAR_WINDOW("ModMenu")) };
Prefs::Bool RomhackMenu{ PrefSection::SECTION_SETTINGS, confPath + "RomhackMenu", false,
                         Synced(CVAR_WINDOW("RomhackMenu")) };
Prefs::Bool SaveEditor{ PrefSection::SECTION_SETTINGS, confPath + "SaveEditor", false,
                        Synced(CVAR_WINDOW("SaveEditor")) };
Prefs::Bool GameplayTools{ PrefSection::SECTION_SETTINGS, confPath + "GameplayTools", false,
                           Synced(CVAR_WINDOW("GameplayTools")) };
Prefs::Bool CheckTracker{ PrefSection::SECTION_SETTINGS, confPath + "CheckTracker", false,
                          Synced(CVAR_WINDOW("CheckTracker")) };
Prefs::Bool CheckTrackerSettings{ PrefSection::SECTION_SETTINGS, confPath + "CheckTrackerSettings", false,
                                  Synced(CVAR_WINDOW("CheckTrackerSettings")) };
Prefs::Bool WorldTracker{ PrefSection::SECTION_SETTINGS, confPath + "WorldTracker", false,
                          Synced(CVAR_WINDOW("WorldTracker")) };
Prefs::Bool WorldTrackerSettings{ PrefSection::SECTION_SETTINGS, confPath + "WorldTrackerSettings", false,
                                  Synced(CVAR_WINDOW("WorldTrackerSettings")) };
Prefs::Bool InputViewer{ PrefSection::SECTION_SETTINGS, confPath + "InputViewer", false,
                         Synced(CVAR_WINDOW("InputViewer")) };
Prefs::Bool InputViewerSettings{ PrefSection::SECTION_SETTINGS, confPath + "InputViewerSettings", false,
                                 Synced(CVAR_WINDOW("InputViewerSettings")) };
Prefs::Bool EventDebugger{ PrefSection::SECTION_SETTINGS, confPath + "EventDebugger", false,
                           Synced(CVAR_WINDOW("EventDebugger")) };
Prefs::Bool OcclusionDebug{ PrefSection::SECTION_SETTINGS, confPath + "OcclusionDebug", false,
                            Synced(CVAR_WINDOW("OcclusionDebug")) };
Prefs::Bool AnchorRoom{ PrefSection::SECTION_SETTINGS, confPath + "AnchorRoom", false,
                        Synced(CVAR_WINDOW("AnchorRoom")) };
// Match CVAR_STATS_WINDOW_OPEN and CVAR_CONSOLE_WINDOW_OPEN via lus-cvars.cmake.
Prefs::Bool Stats{ PrefSection::SECTION_SETTINGS, confPath + "Stats", false, Synced(CVAR_WINDOW("Stats")) };
Prefs::Bool Console{ PrefSection::SECTION_SETTINGS, confPath + "Console", false, Synced(CVAR_WINDOW("Console")) };

} // namespace Windows

} // namespace Prefs::Settings
