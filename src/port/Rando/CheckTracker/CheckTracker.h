#ifndef RANDO_CHECK_TRACKER_H
#define RANDO_CHECK_TRACKER_H

#include <ship/window/gui/GuiWindow.h>
#include <nlohmann/json.hpp>

void CheckTracker_AddToCheckCount(uint32_t randoCheckId);

namespace Rando {

namespace CheckTracker {

void Init();
void LoadFromPreset(const nlohmann::json& info);

class CheckTrackerWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};
};

class SettingsWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};
};

} // namespace CheckTracker

} // namespace Rando

// Prefs
#include "port/Prefs/Pref.h"

namespace Prefs::Trackers::CheckTracker {

extern Prefs::Bool Floating;
extern Prefs::Bool ShowCurrentLevel;
extern Prefs::Bool HideCompletedWorld;
extern Prefs::Bool ShowCollectedChecks;
extern Prefs::Bool ShowWorldChecks;
extern Prefs::Bool ShowLogic;
extern Prefs::Bool SeparateCollectedChecks;
extern Prefs::Bool HideCollected;
extern Prefs::Bool HideSkipped;

extern Prefs::Fixed Opacity;
extern Prefs::Fixed Scale;
extern Prefs::Fixed CollectedChecksOpacity;
extern Prefs::Fixed CollectedChecksScale;

extern Prefs::Color LogicColor;
extern Prefs::Color CollectedColor;
extern Prefs::Color SkippedColor;
extern Prefs::Color ItemColor;

} // namespace Prefs::Trackers::CheckTracker

#endif // RANDO_CHECK_TRACKER_H