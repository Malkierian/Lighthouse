#include "Registry.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <system_error>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <libultraship/libultraship.h>

#include "Pref.h"

namespace Prefs {

namespace {

constexpr const char* kFileName = "lighthouse.settings.json";
constexpr int kDocumentVersion = 1;

bool sDirty = false;
bool sLoaded = false;

std::string FilePath() {
    return Ship::Context::GetPathRelativeToAppDirectory(kFileName);
}

// Both containers are intentionally leaked. Prefs have static lifetime, so a registry
// destroyed first would leave their destructors touching freed memory at exit.
nlohmann::json& Doc() {
    static nlohmann::json* doc = new nlohmann::json(nlohmann::json::object());
    return *doc;
}

nlohmann::json::json_pointer PointerFor(const Base& pref) {
    std::string pointer = "/";
    pointer += SectionBlock(pref.Section());

    const std::string& path = pref.Path();
    size_t start = 0;
    while (start <= path.size()) {
        const size_t dot = path.find('.', start);
        const size_t end = (dot == std::string::npos) ? path.size() : dot;
        pointer += "/";
        pointer.append(path, start, end - start);
        if (dot == std::string::npos) {
            break;
        }
        start = dot + 1;
    }

    return nlohmann::json::json_pointer(pointer);
}

} // namespace

std::vector<Base*>& AllSettings() {
    static std::vector<Base*>* all = new std::vector<Base*>();
    return *all;
}

nlohmann::json& Document() {
    return Doc();
}

void MarkDirty() {
    sDirty = true;
}

bool IsDirty() {
    return sDirty;
}

void StoreNode(const Base& pref) {
    pref.Write(Doc()[PointerFor(pref)]);
}

void EraseNode(const Base& pref) {
    const auto pointer = PointerFor(pref);
    if (!Doc().contains(pointer)) {
        return;
    }

    // json_pointer has no erase; drop the leaf from its parent object.
    const auto parentPointer = pointer.parent_pointer();
    const std::string leaf = pointer.back();
    nlohmann::json& parent = Doc()[parentPointer];
    if (parent.is_object()) {
        parent.erase(leaf);
    }
}

void Load() {
    const std::string path = FilePath();
    bool existed = false;

    std::ifstream file(path);
    if (file.is_open()) {
        existed = true;
        try {
            nlohmann::json parsed;
            file >> parsed;
            if (parsed.is_object()) {
                Doc() = std::move(parsed);
            } else {
                SPDLOG_WARN("Settings file '{}' is not a JSON object; starting fresh", path);
            }
        } catch (const std::exception& e) {
            file.close();
            std::error_code ec;
            const std::string backup = path + ".bak";
            std::filesystem::rename(path, backup, ec);
            SPDLOG_ERROR("Failed to parse '{}': {} — starting fresh; previous file kept as '{}'", path, e.what(),
                         ec ? "(backup failed)" : backup);
        }
        if (file.is_open()) {
            file.close();
        }
    }

    Doc()["version"] = kDocumentVersion;

    for (Base* pref : AllSettings()) {
        const auto pointer = PointerFor(*pref);
        if (!Doc().contains(pointer)) {
            continue;
        }
        pref->ApplyLoaded(Doc()[pointer]);
    }

    sLoaded = true;
    // On a fresh install nothing has been set, so without this the file would never appear
    // until the user changed something. Write a baseline on the first flush instead.
    sDirty = !existed;
}

void Save() {
    if (!sLoaded) {
        return;
    }

    const std::string path = FilePath();
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        SPDLOG_ERROR("Could not open '{}' for writing", path);
        return;
    }

    file << Doc().dump(4) << std::endl;
    file.close();
    sDirty = false;
}

void FlushIfDirty() {
    if (sDirty) {
        Save();
    }
}

Base* Find(PrefSection section, const std::string& path) {
    const auto it = std::find_if(AllSettings().begin(), AllSettings().end(), [section, &path](const Base* pref) {
        return pref->Section() == section && pref->Path() == path;
    });
    return it != AllSettings().end() ? *it : nullptr;
}

Base* FindByCVar(const std::string& cvar) {
    const auto it = std::find_if(AllSettings().begin(), AllSettings().end(),
                                 [&cvar](const Base* pref) { return pref->CVar() != nullptr && cvar == pref->CVar(); });
    return it != AllSettings().end() ? *it : nullptr;
}

} // namespace Prefs
