#include "port/Settings/Settings.h"
#include "port/Settings/Settings_c.h"

#include <cstring>

#include <libultraship/bridge/consolevariablebridge.h>
#include <libultraship/libultraship.h>
#include <ship/window/gui/Gui.h>
#include <spdlog/spdlog.h>

#include "port/ShipInit.hpp"

namespace Settings {

constinit Base* const gAll[SETTING_COUNT] = {
#define SETTING_INT(id, name, def, section) &id,
#define SETTING_FLOAT(id, name, def, section) &id,
#define SETTING_STRING(id, name, def, section) &id,
#define SETTING_COLOR(id, name, r, g, b, a, section) &id,
#define SETTING_COLOR24(id, name, r, g, b, section) &id,
#include "port/Settings/defs/All.inc"
};

Base* Get(SettingId id) {
    return (id >= 0 && id < SETTING_COUNT) ? gAll[id] : nullptr;
}

void Init() {
    for (Base* setting : gAll) {
        setting->Prime();
    }
}

void RefreshAll() {
    for (Base* setting : gAll) {
        setting->Refresh();
    }
}

// Deferred and coalesced by the GUI, so many changes in one frame cost one write. Guarded
// because a setting can legitimately be written before the window exists.
static void RequestConfigSave() {
    auto* context = Ship::Context::GetRawInstance();
    if (context == nullptr) {
        return;
    }
    auto window = context->GetWindow();
    if (window == nullptr) {
        return;
    }
    auto gui = window->GetGui();
    if (gui == nullptr) {
        return;
    }
    gui->SaveConsoleVariablesNextFrame();
}

// The single post-change hook.
//
// Direct writes (Set*/Reset) own the save and the ShipInit call, matching what the widgets
// used to do themselves. Store-sourced writes do not: whoever touched the store already
// handled both — the widget calls ShipInit::Init(cvarName) itself, applyPreset finishes with
// ShipInit::InitAll(), the console's SetCommand saves. Repeating them here would double-fire
// every registered init func.
//
// mOnChange fires either way, since that is the part nothing else covers.
void Base::Notify(ChangeSource source) {
    if (source == ChangeSource::Direct) {
        RequestConfigSave();
        ShipInit::Init(mCvar);
    }
    if (mOnChange != nullptr) {
        mOnChange(*this);
    }
}

bool Base::LoadFromStore() {
    const Value previous = mValue;
    // Not CVarExists() — libultraship declares it but never defines it.
    mSet = CVarGet(mCvar) != nullptr;
    switch (mType) {
        case Type::Int:
            mValue.i = CVarGetInteger(mCvar, mDefault.i);
            return previous.i != mValue.i;
        case Type::Float:
            mValue.f = CVarGetFloat(mCvar, mDefault.f);
            return previous.f != mValue.f;
        case Type::String:
            mValue.s = CVarGetString(mCvar, mDefault.s);
            // Pointer comparison only. A store reload destroys the old CVar objects, so
            // previous.s may already dangle and must not be dereferenced. Worst case this
            // reports a spurious change and fires the callback once too often.
            return previous.s != mValue.s;
        case Type::Color:
            mValue.c = CVarGetColor(mCvar, mDefault.c);
            return std::memcmp(&previous.c, &mValue.c, sizeof(Color_RGBA8)) != 0;
        case Type::Color24:
            mValue.c24 = CVarGetColor24(mCvar, mDefault.c24);
            return std::memcmp(&previous.c24, &mValue.c24, sizeof(Color_RGB8)) != 0;
    }
    return false;
}

void Base::Prime() {
    (void)LoadFromStore();
}

void Base::Refresh() {
    if (LoadFromStore()) {
        Notify(ChangeSource::Store);
    }
}

void Base::Reset() {
    const bool changed = !ValueEquals(mType, mValue, mDefault);
    CVarClear(mCvar);
    mValue = mDefault;
    mSet = false;
    if (changed) {
        Notify(ChangeSource::Direct);
    }
}

bool Base::ValueEquals(Type type, const Value& a, const Value& b) {
    switch (type) {
        case Type::Int:
            return a.i == b.i;
        case Type::Float:
            return a.f == b.f;
        case Type::String:
            if (a.s == b.s) {
                return true;
            }
            return a.s != nullptr && b.s != nullptr && std::strcmp(a.s, b.s) == 0;
        case Type::Color:
            return std::memcmp(&a.c, &b.c, sizeof(Color_RGBA8)) == 0;
        case Type::Color24:
            return std::memcmp(&a.c24, &b.c24, sizeof(Color_RGB8)) == 0;
    }
    return true;
}

bool Base::IsDefault() const {
    return ValueEquals(mType, mValue, mDefault);
}

// Wrong-type access can only come from the C shim, where the id and the accessor are
// picked independently. Report it and fall back rather than reading a dead union member.
static bool CheckType(const Base& setting, Type expected, const char* what) {
    if (setting.GetType() == expected) {
        return true;
    }
    SPDLOG_ERROR("Setting '{}' accessed as the wrong type ({})", setting.Cvar(), what);
    return false;
}

int32_t Base::AsInt() const {
    return CheckType(*this, Type::Int, "int") ? mValue.i : 0;
}

float Base::AsFloat() const {
    return CheckType(*this, Type::Float, "float") ? mValue.f : 0.0f;
}

const char* Base::AsString() const {
    return CheckType(*this, Type::String, "string") ? mValue.s : "";
}

Color_RGBA8 Base::AsColor() const {
    return CheckType(*this, Type::Color, "color") ? mValue.c : Color_RGBA8{ 0, 0, 0, 0 };
}

Color_RGB8 Base::AsColor24() const {
    return CheckType(*this, Type::Color24, "color24") ? mValue.c24 : Color_RGB8{ 0, 0, 0 };
}

void Base::SetInt(int32_t value) {
    if (!CheckType(*this, Type::Int, "int")) {
        return;
    }
    const bool changed = mValue.i != value;
    mValue.i = value;
    mSet = true;
    CVarSetInteger(mCvar, value);
    if (changed) {
        Notify(ChangeSource::Direct);
    }
}

void Base::SetFloat(float value) {
    if (!CheckType(*this, Type::Float, "float")) {
        return;
    }
    const bool changed = mValue.f != value;
    mValue.f = value;
    mSet = true;
    CVarSetFloat(mCvar, value);
    if (changed) {
        Notify(ChangeSource::Direct);
    }
}

void Base::SetString(const char* value) {
    if (!CheckType(*this, Type::String, "string")) {
        return;
    }
    const char* previous = mValue.s;
    const bool changed = previous == nullptr || value == nullptr || std::strcmp(previous, value) != 0;
    CVarSetString(mCvar, value);
    // Re-read so the cache points at the store's copy rather than the caller's buffer.
    mValue.s = CVarGetString(mCvar, mDefault.s);
    mSet = true;
    if (changed) {
        Notify(ChangeSource::Direct);
    }
}

void Base::SetColor(Color_RGBA8 value) {
    if (!CheckType(*this, Type::Color, "color")) {
        return;
    }
    const bool changed = std::memcmp(&mValue.c, &value, sizeof(Color_RGBA8)) != 0;
    mValue.c = value;
    mSet = true;
    CVarSetColor(mCvar, value);
    if (changed) {
        Notify(ChangeSource::Direct);
    }
}

void Base::SetColor24(Color_RGB8 value) {
    if (!CheckType(*this, Type::Color24, "color24")) {
        return;
    }
    const bool changed = std::memcmp(&mValue.c24, &value, sizeof(Color_RGB8)) != 0;
    mValue.c24 = value;
    mSet = true;
    CVarSetColor24(mCvar, value);
    if (changed) {
        Notify(ChangeSource::Direct);
    }
}

} // namespace Settings

extern "C" {

int32_t Setting_GetInt(SettingId id) {
    Settings::Base* setting = Settings::Get(id);
    return setting != nullptr ? setting->AsInt() : 0;
}

float Setting_GetFloat(SettingId id) {
    Settings::Base* setting = Settings::Get(id);
    return setting != nullptr ? setting->AsFloat() : 0.0f;
}

const char* Setting_GetString(SettingId id) {
    Settings::Base* setting = Settings::Get(id);
    return setting != nullptr ? setting->AsString() : "";
}

void Setting_SetInt(SettingId id, int32_t value) {
    if (Settings::Base* setting = Settings::Get(id)) {
        setting->SetInt(value);
    }
}

void Setting_SetFloat(SettingId id, float value) {
    if (Settings::Base* setting = Settings::Get(id)) {
        setting->SetFloat(value);
    }
}

void Setting_SetString(SettingId id, const char* value) {
    if (Settings::Base* setting = Settings::Get(id)) {
        setting->SetString(value);
    }
}

bool Setting_IsDefault(SettingId id) {
    Settings::Base* setting = Settings::Get(id);
    return setting != nullptr ? setting->IsDefault() : true;
}

bool Setting_IsExplicitlySet(SettingId id) {
    Settings::Base* setting = Settings::Get(id);
    return setting != nullptr && setting->IsExplicitlySet();
}

void Setting_Reset(SettingId id) {
    if (Settings::Base* setting = Settings::Get(id)) {
        setting->Reset();
    }
}

} // extern "C"
