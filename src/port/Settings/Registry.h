#pragma once

#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "port/Settings/Sections.h"

namespace Settings {

class Base;

std::vector<Base*>& AllSettings();
void Load();
void Save();
void FlushIfDirty();
void MarkDirty();
bool IsDirty();

Base* Find(SettingSection section, const std::string& path);
Base* FindByCVar(const std::string& cvar);

void StoreNode(const Base& setting);
void EraseNode(const Base& setting);
nlohmann::json& Document();

} // namespace Settings
