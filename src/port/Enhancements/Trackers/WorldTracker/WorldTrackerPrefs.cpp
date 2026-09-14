#include "WorldTracker.h"

namespace Prefs::Trackers::WorldTracker {

std::string confPath = "WorldTracker.";

Prefs::Bool ShowCurrentLevel{ PrefSection::SECTION_TRACKERS, confPath + "ShowCurrentLevel", false,
                              Prefs::Options<bool>().CVar("gRando.WorldTracker.ShowCurrentLevel") };
Prefs::Bool ShowTotalCollected{ PrefSection::SECTION_TRACKERS, confPath + "ShowTotalCollected", false,
                                Prefs::Options<bool>().CVar("gRando.WorldTracker.ShowTotalCollected") };
Prefs::Bool SeparateTotals{ PrefSection::SECTION_TRACKERS, confPath + "SeparateTotals", false };

} // namespace Prefs::Trackers::WorldTracker
