#include "Pref.h"

#include <algorithm>
#include <random>

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

Base::Base(PrefSection section, std::string path, const char* cvar)
    : mSection(section), mPath(std::move(path)), mCvar(cvar) {
    mFullPath = std::string(SectionBlock(mSection)) + "." + mPath;
    AllSettings().push_back(this);
}

Base::~Base() {
    auto& all = AllSettings();
    all.erase(std::remove(all.begin(), all.end(), this), all.end());
}

void Base::MarkChanged(bool valueChanged) {
    StoreNode(*this);
    MarkDirty();

    if (!valueChanged) {
        return;
    }

    ShipInit::Init(mFullPath);

    if (mOnChange) {
        mOnChange(*this);
    }
}

void Base::MarkReset(bool valueChanged) {
    EraseNode(*this);
    MarkDirty();

    if (!valueChanged) {
        return;
    }

    ShipInit::Init(mFullPath);

    if (mOnChange) {
        mOnChange(*this);
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
