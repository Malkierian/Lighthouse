#include "Pref.h"

#include <algorithm>
#include <cmath>
#include <random>

#include <libultraship/libultraship.h>

#include "Registry.h"
#include "Pref_impl.h"
#include "port/ShipInit.hpp"

namespace Prefs {

namespace {
constexpr const char* kSectionBlocks[SECTION_COUNT] = {
    "settings", "enhancements", "rando", "trackers", "network", // "cosmetics", "audio",
};
} // namespace

const char* SectionBlock(PrefSection section) {
    return section < SECTION_COUNT ? kSectionBlocks[section] : "";
}

Base::Base(PrefSection section, std::string path, const char* cvar, bool syncCvar)
    : mSection(section), mPath(std::move(path)), mCvar(cvar), mSyncCvar(syncCvar) {
    mFullPath = std::string(SectionBlock(mSection)) + "." + mPath;
    AllSettings().push_back(this);
    if (mSyncCvar) {
        SyncedSettings().push_back(this);
    }
}

Base::~Base() {
    auto& all = AllSettings();
    all.erase(std::remove(all.begin(), all.end(), this), all.end());
    auto& synced = SyncedSettings();
    synced.erase(std::remove(synced.begin(), synced.end(), this), synced.end());
}

void Base::MarkChanged(bool valueChanged) {
    if (StoreNode(*this)) {
        MarkDirty();
    }
    WriteCVar();

    if (!valueChanged) {
        return;
    }

    ShipInit::Init(mFullPath);

    if (mOnChange) {
        mOnChange(*this);
    }
}

void Base::MarkReset(bool valueChanged) {
    if (EraseNode(*this)) {
        MarkDirty();
    }
    WriteCVar();

    if (!valueChanged) {
        return;
    }

    ShipInit::Init(mFullPath);

    if (mOnChange) {
        mOnChange(*this);
    }
}

void Base::MarkLoaded() {
    if (StoreNode(*this)) {
        MarkDirty();
    }
}

void Color::SetValue(Color_RGBA8 value) {
    ColorValue next = mValue;
    next.value = value;
    Set(next);
}

void Color::SetRainbow(bool rainbow) {
    ColorValue next = mValue;
    next.rainbow = rainbow;
    Set(next);
}

void Color::SetLocked(bool locked) {
    ColorValue next = mValue;
    next.locked = locked;
    Set(next);
}

void Color::Randomize() {
#if !defined(__SWITCH__) && !defined(__WIIU__)
    std::random_device rd;
    std::mt19937 rng(rd());
#else
    std::mt19937 rng(static_cast<uint32_t>(std::hash<std::string>{}(std::to_string(rand()))));
#endif
    std::uniform_int_distribution<int> dist(0, 255);

    ColorValue next = mValue;
    next.value.r = static_cast<uint8_t>(dist(rng));
    next.value.g = static_cast<uint8_t>(dist(rng));
    next.value.b = static_cast<uint8_t>(dist(rng));
    next.rainbow = false; // A picked color and a cycling one are mutually exclusive.
    Set(next);
}

Enum::Enum(PrefSection section, std::string path, int32_t def, const std::map<int32_t, EnumEntry>& entries,
           Options<int32_t> options)
    : Scalar<int32_t>(section, std::move(path), def, std::move(options)), mEntries(&entries) {
    InstallEntryValidator();
}

Enum::Enum(PrefSection section, std::string path, int32_t def, std::map<int32_t, EnumEntry>&& entries,
           Options<int32_t> options)
    : Scalar<int32_t>(section, std::move(path), def, std::move(options)), mOwnedEntries(std::move(entries)),
      mEntries(&*mOwnedEntries) {
    InstallEntryValidator();
}

// Keys validate against the map itself rather than a copied allow-list, so sharing a map costs
// nothing per pref and the lookup happens after construction.
void Enum::InstallEntryValidator() {
    if (mOptions.validator) {
        return;
    }
    mOptions.validator = [this](int32_t& value) { return mEntries->contains(value); };
}

void Enum::Write(nlohmann::json& out) const {
    const auto it = mEntries->find(mValue);
    if (it != mEntries->end()) {
        out = it->second.wireName;
        return;
    }
    if (!mPendingWireName.empty()) {
        out = mPendingWireName;
        return;
    }
    out = mValue;
}

bool Enum::Read(const nlohmann::json& in) {
    if (in.is_string()) {
        const std::string name = in.get<std::string>();
        for (const auto& [key, entry] : *mEntries) {
            if (name == entry.wireName) {
                mValue = key;
                mPendingWireName.clear();
                return true;
            }
        }
        mPendingWireName = name;
        SPDLOG_WARN("Setting '{}' has no entry named '{}'; using the default and keeping the stored name", mPath, name);
        return false;
    }

    if (in.is_number_integer()) {
        mPendingWireName.clear();
        return Scalar<int32_t>::Read(in);
    }

    SPDLOG_WARN("Setting '{}' is neither an entry name nor a key in the config; using the default", mPath);
    return false;
}

// No Write/Read override: the config sees the plain int32 Scalar<int32_t> already serialises.
Fixed::Fixed(PrefSection section, std::string path, int32_t def, int32_t factor, Options<int32_t> options)
    : Scalar<int32_t>(section, std::move(path), def, std::move(options)), mFactor(factor < 1 ? 1 : factor) {
}

int32_t Fixed::Decimals() const {
    int32_t places = 0;
    for (int32_t f = mFactor; f >= 10; f /= 10) {
        ++places;
    }
    return places;
}

void Fixed::SetFloat(float value) {
    Set((int32_t)std::lround(value * (float)mFactor));
}

void Fixed::WriteCVar() const {
    if (mSyncCvar) {
        detail::WriteCVar(mCvar, Float());
    }
}

bool Fixed::AdoptCVar(bool live) {
    if (mCvar == nullptr) {
        return false;
    }
    float value = Float();
    const CVarRead result = detail::ReadCVar(mCvar, value);
    return ApplyCVarRead(result, (int32_t)std::lround(value * (float)mFactor), live);
}

namespace detail {

namespace {
auto Vars() {
    auto* context = Ship::Context::GetRawInstance();
    return context != nullptr ? context->GetConsoleVariables() : nullptr;
}

auto FindCVar(const char* name) {
    const auto vars = Vars();
    return vars != nullptr ? vars->Get(name) : nullptr;
}
} // namespace

CVarRead ReadCVar(const char* name, bool& out) {
    int32_t value = out ? 1 : 0;
    const CVarRead result = ReadCVar(name, value);
    if (result == CVarRead::Ok) {
        out = value != 0;
    }
    return result;
}

CVarRead ReadCVar(const char* name, int32_t& out) {
    const auto cvar = FindCVar(name);
    if (cvar == nullptr) {
        return CVarRead::Absent;
    }
    using Type = decltype(cvar->Type);
    if (cvar->Type == Type::Integer) {
        out = cvar->Integer;
    } else if (cvar->Type == Type::Float) {
        out = (int32_t)std::lround(cvar->Float);
    } else {
        return CVarRead::Unusable;
    }
    return CVarRead::Ok;
}

CVarRead ReadCVar(const char* name, float& out) {
    const auto cvar = FindCVar(name);
    if (cvar == nullptr) {
        return CVarRead::Absent;
    }
    using Type = decltype(cvar->Type);
    if (cvar->Type == Type::Float) {
        out = cvar->Float;
    } else if (cvar->Type == Type::Integer) {
        out = (float)cvar->Integer;
    } else {
        return CVarRead::Unusable;
    }
    return CVarRead::Ok;
}

CVarRead ReadCVar(const char* name, std::string& out) {
    const auto cvar = FindCVar(name);
    if (cvar == nullptr) {
        return CVarRead::Absent;
    }
    using Type = decltype(cvar->Type);
    if (cvar->Type != Type::String || cvar->String == nullptr) {
        return CVarRead::Unusable;
    }
    out = cvar->String;
    return CVarRead::Ok;
}

CVarRead ReadCVar(const char* name, Color_RGBA8& out) {
    const auto cvar = FindCVar(name);
    if (cvar == nullptr) {
        return CVarRead::Absent;
    }
    using Type = decltype(cvar->Type);
    if (cvar->Type == Type::Color) {
        out = cvar->Color;
    } else if (cvar->Type == Type::Color24) {
        out = { cvar->Color24.r, cvar->Color24.g, cvar->Color24.b, 255 };
    } else {
        return CVarRead::Unusable;
    }
    return CVarRead::Ok;
}

// CVarColorPicker's layout.
CVarRead ReadCVar(const char* name, ColorValue& out) {
    const std::string base = name;
    ColorValue read = out;
    const CVarRead results[] = {
        ReadCVar((base + ".Value").c_str(), read.value),
        ReadCVar((base + ".Rainbow").c_str(), read.rainbow),
        ReadCVar((base + ".Locked").c_str(), read.locked),
    };
    bool anyPresent = false;
    for (const CVarRead result : results) {
        if (result == CVarRead::Unusable) {
            return CVarRead::Unusable;
        }
        anyPresent |= result == CVarRead::Ok;
    }
    if (!anyPresent) {
        return CVarRead::Absent;
    }
    out = read;
    return CVarRead::Ok;
}

void WriteCVar(const char* name, bool value) {
    WriteCVar(name, static_cast<int32_t>(value ? 1 : 0));
}

void WriteCVar(const char* name, int32_t value) {
    if (const auto vars = Vars()) {
        vars->SetInteger(name, value);
    }
}

void WriteCVar(const char* name, float value) {
    if (const auto vars = Vars()) {
        vars->SetFloat(name, value);
    }
}

void WriteCVar(const char* name, const std::string& value) {
    if (const auto vars = Vars()) {
        vars->SetString(name, value.c_str());
    }
}

void WriteCVar(const char* name, const Color_RGBA8& value) {
    if (const auto vars = Vars()) {
        vars->SetColor(name, value);
    }
}

} // namespace detail

// Emitted here so Setting.h can stay on <nlohmann/json_fwd.hpp>. A module needing some other
// V includes Setting_impl.h from its own .cpp and pays the parse cost alone.
template class Scalar<bool>;
template class Scalar<int32_t>;
template class Scalar<float>;
template class Scalar<std::string>;
template class Scalar<Color_RGBA8>;
template class Scalar<ColorValue>;
template class Scalar<std::vector<int32_t>>;
template class Scalar<std::vector<float>>;
template class Scalar<std::vector<std::string>>;
template class Scalar<std::map<std::string, int32_t>>;
template class Scalar<std::map<std::string, float>>;
template class Scalar<std::map<std::string, std::string>>;

} // namespace Prefs
