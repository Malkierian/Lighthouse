#pragma once

#include <stdint.h>

#include "libultraship/color.h"
#include "port/Presets/Presets.h"

// Deliberately does NOT include SettingIds.h. The classes here are independent of which
// settings exist, so editing a defs/*.inc fragment does not dirty anything that only needs
// the types (MenuTypes.h, UIWidgets). Only Settings.h and the C shim depend on the table.

namespace Settings {

enum class Type : uint8_t { Int, Float, String, Color, Color24 };

// Storage and behaviour shared by every setting. Deliberately a literal type —
// POD members, constexpr constructor, no virtuals — so the declarations in
// Settings.h can be constinit and there is no static-init order to reason about.
// Base owns the value union, so the typed classes below add no storage and the
// C shim can work through a Base* without downcasting.
class Base {
  public:
    union Value {
        int32_t i;
        float f;
        const char* s;
        Color_RGBA8 c;
        Color_RGB8 c24;

        constexpr Value(int32_t v) : i(v) {
        }
        constexpr Value(float v) : f(v) {
        }
        constexpr Value(const char* v) : s(v) {
        }
        constexpr Value(Color_RGBA8 v) : c(v) {
        }
        constexpr Value(Color_RGB8 v) : c24(v) {
        }
    };

    // Raw SettingId value. Typed as uint16_t rather than SettingId so this header stays
    // independent of the generated enum; cast at the call site if you need the enum type.
    uint16_t Id() const {
        return mId;
    }
    const char* Cvar() const {
        return mCvar;
    }
    Type GetType() const {
        return mType;
    }
    // PRESET_SECTION_MAX means the setting belongs to no preset block.
    PresetSection Section() const {
        return mSection;
    }

    // True when the value exists in the CVar store rather than falling back to the default.
    bool IsExplicitlySet() const {
        return mSet;
    }
    bool IsDefault() const;

    // Restore the compiled-in default and drop the persisted entry. Fires OnChange if
    // the value actually moved.
    void Reset();
    // Re-read from the CVar store, firing OnChange if the value moved. This is what makes
    // callbacks work for changes that bypass the typed API — presets, the console, direct
    // CVarSet calls.
    void Refresh();
    // Re-read without ever firing OnChange. Used once at startup, before the subsystems a
    // callback would poke exist.
    void Prime();

    // Fires whenever the value changes, no matter who changed it. Must be capture-less:
    // Base stays a literal type so the declarations in Settings.h can be constinit, which
    // rules out std::function. The changed setting is passed in, so context is rarely needed.
    // One callback per setting; registering again replaces the previous one.
    using ChangeCallback = void (*)(Base&);
    void SetOnChange(ChangeCallback callback) {
        mOnChange = callback;
    }

    // Type-guarded raw accessors. The typed classes below are the intended API; these
    // exist for the C shim, which only has a SettingId to work with.
    int32_t AsInt() const;
    float AsFloat() const;
    const char* AsString() const;
    Color_RGBA8 AsColor() const;
    Color_RGB8 AsColor24() const;

    void SetInt(int32_t value);
    void SetFloat(float value);
    void SetString(const char* value);
    void SetColor(Color_RGBA8 value);
    void SetColor24(Color_RGB8 value);

  protected:
    constexpr Base(uint16_t id, const char* cvar, Type type, PresetSection section, Value def)
        : mId(id), mCvar(cvar), mType(type), mSection(section), mDefault(def), mValue(def) {
    }

    // Direct = this object wrote the store (Set*/Reset), so it owns the save and the
    // ShipInit call. Store = the store was written by someone else who already handled
    // both; only mOnChange is ours to fire.
    enum class ChangeSource { Direct, Store };
    void Notify(ChangeSource source);
    // Pulls the store into mValue/mSet. Returns whether the value moved.
    bool LoadFromStore();
    // Full comparison, including strcmp for strings. Only safe when both operands are live —
    // see the string note in LoadFromStore.
    static bool ValueEquals(Type type, const Value& a, const Value& b);

    uint16_t mId;
    const char* mCvar;
    Type mType;
    PresetSection mSection;
    Value mDefault;
    Value mValue;
    void (*mOnChange)(Base&) = nullptr;
    bool mSet = false;
};

class Int : public Base {
  public:
    constexpr Int(uint16_t id, const char* cvar, int32_t def, PresetSection section)
        : Base(id, cvar, Type::Int, section, Value(def)) {
    }

    int32_t Get() const {
        return mValue.i;
    }
    bool GetBool() const {
        return mValue.i != 0;
    }
    operator int32_t() const {
        return mValue.i;
    }
    int32_t Default() const {
        return mDefault.i;
    }

    void Set(int32_t value) {
        SetInt(value);
    }
    Int& operator=(int32_t value) {
        SetInt(value);
        return *this;
    }
    Int& operator=(const Int&) = delete;
};

class Float : public Base {
  public:
    constexpr Float(uint16_t id, const char* cvar, float def, PresetSection section)
        : Base(id, cvar, Type::Float, section, Value(def)) {
    }

    float Get() const {
        return mValue.f;
    }
    operator float() const {
        return mValue.f;
    }
    float Default() const {
        return mDefault.f;
    }

    void Set(float value) {
        SetFloat(value);
    }
    Float& operator=(float value) {
        SetFloat(value);
        return *this;
    }
    Float& operator=(const Float&) = delete;
};

// The cached pointer is owned by the CVar store, not by the setting — Set() writes
// through and then re-reads so we never hold the caller's buffer.
class String : public Base {
  public:
    constexpr String(uint16_t id, const char* cvar, const char* def, PresetSection section)
        : Base(id, cvar, Type::String, section, Value(def)) {
    }

    const char* Get() const {
        return mValue.s;
    }
    operator const char*() const {
        return mValue.s;
    }
    const char* Default() const {
        return mDefault.s;
    }

    void Set(const char* value) {
        SetString(value);
    }
    String& operator=(const char* value) {
        SetString(value);
        return *this;
    }
    String& operator=(const String&) = delete;
};

class Color : public Base {
  public:
    constexpr Color(uint16_t id, const char* cvar, Color_RGBA8 def, PresetSection section)
        : Base(id, cvar, Type::Color, section, Value(def)) {
    }

    Color_RGBA8 Get() const {
        return mValue.c;
    }
    operator Color_RGBA8() const {
        return mValue.c;
    }
    Color_RGBA8 Default() const {
        return mDefault.c;
    }

    void Set(Color_RGBA8 value) {
        SetColor(value);
    }
    Color& operator=(Color_RGBA8 value) {
        SetColor(value);
        return *this;
    }
    Color& operator=(const Color&) = delete;
};

class Color24 : public Base {
  public:
    constexpr Color24(uint16_t id, const char* cvar, Color_RGB8 def, PresetSection section)
        : Base(id, cvar, Type::Color24, section, Value(def)) {
    }

    Color_RGB8 Get() const {
        return mValue.c24;
    }
    operator Color_RGB8() const {
        return mValue.c24;
    }
    Color_RGB8 Default() const {
        return mDefault.c24;
    }

    void Set(Color_RGB8 value) {
        SetColor24(value);
    }
    Color24& operator=(Color_RGB8 value) {
        SetColor24(value);
        return *this;
    }
    Color24& operator=(const Color24&) = delete;
};

} // namespace Settings
