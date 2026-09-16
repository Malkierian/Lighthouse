#pragma once

#include "UIWidgets.hpp"
#include "InputViewer.h"
#include "LighthouseModals.h"

namespace LighthouseGui {
void SetupMenu();
void SetupGuiElements();
void Draw();
void Destroy();
void RegisterPopup(std::string title, std::string message, std::string button1 = "OK", std::string button2 = "",
                   std::function<void()> button1callback = nullptr, std::function<void()> button2callback = nullptr);
size_t PopupsQueued();
UIWidgets::Colors GetMenuThemeColor();
} // namespace LighthouseGui

#define THEME_COLOR LighthouseGui::GetMenuThemeColor()

// Prefs

namespace Prefs::Settings {

namespace Windows {
extern Prefs::Bool Menu;
extern Prefs::Bool ControllerConfiguration;
extern Prefs::Bool GamepadMapper;
extern Prefs::Bool ModMenu;
extern Prefs::Bool RomhackMenu;
extern Prefs::Bool SaveEditor;
extern Prefs::Bool GameplayTools;
extern Prefs::Bool CheckTracker;
extern Prefs::Bool CheckTrackerSettings;
extern Prefs::Bool WorldTracker;
extern Prefs::Bool WorldTrackerSettings;
extern Prefs::Bool InputViewer;
extern Prefs::Bool InputViewerSettings;
extern Prefs::Bool EventDebugger;
extern Prefs::Bool OcclusionDebug;
extern Prefs::Bool AnchorRoom;
extern Prefs::Bool Stats;
extern Prefs::Bool Console;
} // namespace Windows

} // namespace Prefs::Settings
