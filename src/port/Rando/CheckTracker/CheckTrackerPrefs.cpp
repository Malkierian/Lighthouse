#include "CheckTracker.h"

namespace Prefs::Trackers::CheckTracker {

std::string confPath = "CheckTracker.";

Prefs::Bool Floating{ PrefSection::SECTION_TRACKERS, confPath + "Floating", false,
                      Prefs::Options<bool>().CVar("gRando.CheckTracker.Floating") };
Prefs::Bool ShowCurrentLevel{ PrefSection::SECTION_TRACKERS, confPath + "ShowCurrentLevel", false,
                              Prefs::Options<bool>().CVar("gRando.CheckTracker.ShowCurrentLevel") };
Prefs::Bool HideCompletedWorld{ PrefSection::SECTION_TRACKERS, confPath + "HideCompletedWorld", false,
                                Prefs::Options<bool>().CVar("gRando.CheckTracker.HideCompletedWorld") };
Prefs::Bool ShowCollectedChecks{ PrefSection::SECTION_TRACKERS, confPath + "ShowCollectedChecks", true,
                                 Prefs::Options<bool>().CVar("gRando.CheckTracker.ShowCollectedChecks") };
Prefs::Bool ShowWorldChecks{ PrefSection::SECTION_TRACKERS, confPath + "ShowWorldChecks", true,
                             Prefs::Options<bool>().CVar("gRando.CheckTracker.ShowWorldChecks") };
Prefs::Bool ShowLogic{ PrefSection::SECTION_TRACKERS, confPath + "ShowLogic", false,
                       Prefs::Options<bool>().CVar("gRando.CheckTracker.ShowLogic") };
Prefs::Bool SeparateCollectedChecks{ PrefSection::SECTION_TRACKERS, confPath + "SeparateCollectedChecks", false,
                                     Prefs::Options<bool>().CVar("gRando.CheckTracker.SeparateCollectedChecks") };
Prefs::Bool HideCollected{ PrefSection::SECTION_TRACKERS, confPath + "HideCollected", false,
                           Prefs::Options<bool>().CVar("gRando.CheckTracker.HideCollected") };
Prefs::Bool HideSkipped{ PrefSection::SECTION_TRACKERS, confPath + "HideSkipped", false,
                         Prefs::Options<bool>().CVar("gRando.CheckTracker.HideSkipped") };

// Hundredths: 0.00 to 1.00.
Prefs::Fixed Opacity{ PrefSection::SECTION_TRACKERS, confPath + "Opacity", 50, 100,
                      Prefs::Options<int32_t>().CVar("gRando.CheckTracker.Opacity").Min(0).Max(100) };
// Tenths: 0.7 to 2.5.
Prefs::Fixed Scale{ PrefSection::SECTION_TRACKERS, confPath + "Scale", 100, 100,
                    Prefs::Options<int32_t>().CVar("gRando.CheckTracker.Scale").Min(70).Max(250) };
Prefs::Fixed CollectedChecksOpacity{
    PrefSection::SECTION_TRACKERS, confPath + "CollectedChecksOpacity", 50, 100,
    Prefs::Options<int32_t>().CVar("gRando.CheckTracker.CollectedChecksOpacity").Min(0).Max(100)
};
Prefs::Fixed CollectedChecksScale{
    PrefSection::SECTION_TRACKERS, confPath + "CollectedChecksScale", 100, 100,
    Prefs::Options<int32_t>().CVar("gRando.CheckTracker.CollectedChecksScale").Min(70).Max(250)
};

Prefs::Color LogicColor{ PrefSection::SECTION_TRACKERS,
                         confPath + "LogicColor",
                         { 200, 200, 200, 255 },
                         Prefs::Options<Prefs::ColorValue>().CVar("gRando.CheckTracker.LogicColor") };
Prefs::Color CollectedColor{ PrefSection::SECTION_TRACKERS,
                             confPath + "CollectedColor",
                             { 100, 255, 100, 255 },
                             Prefs::Options<Prefs::ColorValue>().CVar("gRando.CheckTracker.CollectedColor") };
Prefs::Color SkippedColor{ PrefSection::SECTION_TRACKERS,
                           confPath + "SkippedColor",
                           { 255, 100, 255, 255 },
                           Prefs::Options<Prefs::ColorValue>().CVar("gRando.CheckTracker.SkippedColor") };
Prefs::Color ItemColor{ PrefSection::SECTION_TRACKERS,
                        confPath + "ItemColor",
                        { 79, 0, 221, 255 },
                        Prefs::Options<Prefs::ColorValue>().CVar("gRando.CheckTracker.ItemColor") };

} // namespace Prefs::Trackers::CheckTracker
