#include "InputViewer.h"

#define CVAR_INPUT_VIEWER(var) "gInputViewer." var

namespace Prefs::Settings {

namespace InputViewer {
Prefs::Float Scale { PrefSection::SECTION_SETTINGS, "Scale", 1.0f,
                     Prefs::Options<float>().CVar(CVAR_INPUT_VIEWER("Scale")).Min(0.1f).Max(5.0f)};

namespace AnalogStick {
Prefs::Int32 Movement { PrefSection::SECTION_SETTINGS, "AnalogStick.Movement", 12,
                        Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogStick.Movement")).Min(0).Max(200)};

} // namespace AnalogStick

namespace AnalogAngles {
Prefs::Bool Enabled { PrefSection::SECTION_SETTINGS, "AnalogAngles.Enabled", false,
                      Prefs::Options<bool>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Enabled")) };
Prefs::Int32 Offset { PrefSection::SECTION_SETTINGS, "AnalogAngles.Offset", 7,
                      Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Offset")).Min(0).Max(200)};
Prefs::Float Scale { PrefSection::SECTION_SETTINGS, "AnalogAngles.Scale", 1.0f,
                     Prefs::Options<float>().CVar(CVAR_INPUT_VIEWER("AnalogAngles.Scale")).Min(0.1f).Max(5.0f)};
} // namespace AnalogAngles

namespace RightStick {
Prefs::Int32 Movement { PrefSection::SECTION_SETTINGS, "RightStick.Movement", 0,
                        Prefs::Options<int32_t>().CVar(CVAR_INPUT_VIEWER("RightStick.Movement")).Min(0).Max(400)};
} // namespace RightStick

} // namespace InputViewer

} // namespace Prefs::Settings