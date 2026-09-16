#pragma once

#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "Pref.h"

namespace Prefs {

class Base;

std::vector<Base*>& AllSettings();
std::vector<Base*>& SyncedSettings();
void Load();
void Save();
void FlushIfDirty();
void FlushNow();
void MarkDirty();
bool IsDirty();
void SyncCVars();
void MigrateLegacyCVars();
void ResetAll();

Base* Find(PrefSection section, const std::string& path);
Base* FindByCVar(const std::string& cvar);

bool StoreNode(const Base& pref);
bool EraseNode(const Base& pref);
nlohmann::json& Document();

} // namespace Prefs
