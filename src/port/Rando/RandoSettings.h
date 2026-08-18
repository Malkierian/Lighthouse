#pragma once

// Rando-owned settings. Only Rando consumers include this, so adding one here rebuilds
// nothing outside those files. Defined in RandoSettings.cpp.

#include "port/Settings/Setting.h"

namespace Settings::Rando {

/// Notifications when a shuffled item is collected.
extern Bool Notifications;

/// Manually entered seed; empty means "generate one".
extern String InputSeed;

} // namespace Settings::Rando
