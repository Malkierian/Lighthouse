#pragma once

#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "Pref.h"

namespace Prefs {

class Base;

std::vector<Base*>& AllSettings();
void Load();
void Save();
void FlushIfDirty();
void MarkDirty();
bool IsDirty();

Base* Find(PrefSection section, const std::string& path);
Base* FindByCVar(const std::string& cvar);

void StoreNode(const Base& pref);
void EraseNode(const Base& pref);
nlohmann::json& Document();

} // namespace Prefs
