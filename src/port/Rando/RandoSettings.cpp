#include "port/Rando/RandoSettings.h"

namespace Settings::Rando {

// No Min/Max needed — a Bool has no invalid in-range state, which is half the point of the type.
Bool Notifications{ SECTION_RANDO, "notifications", true, Options<bool>().CVar("gRandoSettings.RandoNotifications") };

String InputSeed{ SECTION_RANDO, "inputSeed", "", Options<std::string>().MaxSize(64) };

} // namespace Settings::Rando
