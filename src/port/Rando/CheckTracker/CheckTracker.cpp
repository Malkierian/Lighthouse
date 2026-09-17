#include "CheckTracker.h"
#include "port/Rando/Logic/Logic.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "port/UI/UIWidgets.hpp"
#include <cstring>
#include "port/GameStatus.h"

extern "C" {
enum map_e level_get_main_map(enum level_e level_id);
enum map_e gsworld_getMap(void);
enum level_e map_getLevel(enum map_e map);
}

namespace LighthouseGui {
extern std::shared_ptr<Rando::CheckTracker::CheckTrackerWindow> mRandoCheckTrackerWindow;
}

std::vector<std::pair<Prefs::Color*, const char*>> checkColorList = {
    { &Prefs::Trackers::CheckTracker::LogicColor, "Out of Logic" },
    { &Prefs::Trackers::CheckTracker::CollectedColor, "Check Collected" },
    { &Prefs::Trackers::CheckTracker::SkippedColor, "Check Skipped" },
    { &Prefs::Trackers::CheckTracker::ItemColor, "Obtained Item" },
};

std::map<RandoCheckId, std::string> checkList;

Rando::StaticData::RandoLogicData reachableRegions[RR_MAX];
Rando::StaticData::RandoLogicData reachableEvents[RA_MAX];
Rando::StaticData::RandoLogicData reachableChecks[RC_MAX];

ImVec4 checkTrackerBG = ImVec4{ 0, 0, 0, 0.5f };
ImVec4 collectedChecksBG = ImVec4{ 0, 0, 0, 0.5f };
float checkTrackerScale = 1.0f;
float collectedChecksScale = 1.0f;

static bool presetLoaded = false;
static ImVec2 presetPos;
static ImVec2 presetSize;

bool expandToggle = true;
bool expandState = true;

ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;

#define WORLD_COUNT 12

uint32_t checkCountTotal;
uint32_t checkCountPerWorld[WORLD_COUNT];
uint32_t checkCollectedCountTotal;
uint32_t checkCollectedCountPerWorld[WORLD_COUNT];

void CheckTracker_InitiateTotals() {
    checkCountTotal = 0;
    checkCollectedCountTotal = 0;

    for (int32_t i = 0; i < WORLD_COUNT; i++) {
        checkCountPerWorld[i] = 0;
        checkCollectedCountPerWorld[i] = 0;
    }

    for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
        RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
        int32_t worldId = Rando::StaticData::Checks[randoCheckId].worldId;
        if (randoSaveCheck.eligible || randoSaveCheck.skipped) {
            checkCollectedCountTotal++;
            checkCollectedCountPerWorld[worldId]++;
        }
        checkCountTotal++;
        checkCountPerWorld[worldId]++;
    }
}

void CheckTracker_AddToCheckCount(uint32_t randoCheckId) {
    RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[(RandoCheckId)randoCheckId];
    int32_t worldId = Rando::StaticData::Checks[(RandoCheckId)randoCheckId].worldId;
    checkCollectedCountTotal++;
    checkCollectedCountPerWorld[worldId]++;
}

void CheckTracker_RemoveFromCheckCount(uint32_t randoCheckId) {
    RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[(RandoCheckId)randoCheckId];
    int32_t worldId = Rando::StaticData::Checks[(RandoCheckId)randoCheckId].worldId;
    checkCollectedCountTotal--;
    checkCollectedCountPerWorld[worldId]--;
}

void CheckTracker_CreateCheckList() {
    checkList.clear();
    for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
        if (RANDO_SAVE_CHECKS[randoCheckId].isShuffled) {
            checkList.insert({ randoCheckId, Ship_ConvertEnumToReadableName(randoStaticCheck.name) });
        }
    }
}

std::string CheckTracker_GetTotalCheckCountString() {
    std::string totalChecks;
    totalChecks = std::to_string(checkCollectedCountTotal);
    totalChecks += " of ";
    totalChecks += std::to_string(checkCountTotal);
    return totalChecks;
}

std::string CheckTracker_GetWorldCheckCountString(level_e world) {
    std::string worldCheckString;
    worldCheckString = std::to_string(checkCollectedCountPerWorld[world]);
    worldCheckString += " / ";
    worldCheckString += std::to_string(checkCountPerWorld[world]);
    return worldCheckString;
}

void CheckTracker_DrawCheckCount() {
    if (Prefs::Trackers::CheckTracker::ShowCollectedChecks) {
        if (Prefs::Trackers::CheckTracker::SeparateCollectedChecks) {
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, collectedChecksBG);
            ImGui::PushStyleColor(ImGuiCol_TitleBg, collectedChecksBG);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, collectedChecksBG);
            if (ImGui::Begin("CheckCount", nullptr,
                             ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                ImGui::SetWindowFontScale(collectedChecksScale);
                ImGui::Text("Checks: %s", CheckTracker_GetTotalCheckCountString().c_str());
                ImGui::End();
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::Text("Checks: %s", CheckTracker_GetTotalCheckCountString().c_str());
        }
    }
}

void DrawCheckTrackerList() {
    if (Prefs::Trackers::CheckTracker::ShowCollectedChecks && !Prefs::Trackers::CheckTracker::SeparateCollectedChecks) {
        CheckTracker_DrawCheckCount();
    }

    for (int i = LEVEL_1_MUMBOS_MOUNTAIN; i < WORLD_COUNT; i++) {

        if (Prefs::Trackers::CheckTracker::ShowCurrentLevel && i != map_getLevel(gsworld_getMap())) {
            continue;
        }

        if (Prefs::Trackers::CheckTracker::HideCompletedWorld &&
            checkCountPerWorld[i] == checkCollectedCountPerWorld[i]) {
            continue;
        }

        std::string headerName = worldNameList[i];
        if (Prefs::Trackers::CheckTracker::ShowWorldChecks) {
            headerName += " ";
            headerName += CheckTracker_GetWorldCheckCountString((level_e)i);
        }

        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0.5f));

        if (expandState != expandToggle) {
            ImGui::SetNextItemOpen(expandToggle);
        }

        if (ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(20.0f);
            if (ImGui::BeginTable("CheckTrackerTable", 1)) {

                ImGui::TableNextColumn();
                for (auto& [randoCheckId, checkString] : checkList) {
                    RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
                    if (Rando::StaticData::Checks[randoCheckId].worldId != i) {
                        continue;
                    }

                    if (Prefs::Trackers::CheckTracker::HideCollected && randoSaveCheck.eligible) {
                        continue;
                    }

                    if (Prefs::Trackers::CheckTracker::HideSkipped && randoSaveCheck.skipped) {
                        continue;
                    }

                    ImVec4 checkTextColor = randoSaveCheck.eligible
                                                ? VecFromRGBA8(Prefs::Trackers::CheckTracker::CollectedColor.Value())
                                                : UIWidgets::ColorValues.at(UIWidgets::Colors::White);

                    ImVec4 itemTextColor = randoSaveCheck.eligible
                                               ? VecFromRGBA8(Prefs::Trackers::CheckTracker::ItemColor.Value())
                                               : UIWidgets::ColorValues.at(UIWidgets::Colors::Indigo);
                    if (randoSaveCheck.skipped) {
                        checkTextColor = itemTextColor =
                            VecFromRGBA8(Prefs::Trackers::CheckTracker::SkippedColor.Value());
                    }

                    if (!randoSaveCheck.eligible && Prefs::Trackers::CheckTracker::ShowLogic) {
                        checkTextColor = Rando::Logic::CanAccessCheck(randoCheckId)
                                             ? UIWidgets::ColorValues.at(UIWidgets::Colors::White)
                                             : VecFromRGBA8(Prefs::Trackers::CheckTracker::LogicColor.Value());
                    }

                    ImGui::BeginGroup();
                    ImGui::TextColored(checkTextColor, checkString.c_str());
                    if (randoSaveCheck.eligible) {
                        ImGui::SameLine();
                        const std::string& randoItemName = Rando::StaticData::Items[randoSaveCheck.randoItemId].name;
                        ImGui::TextColored(itemTextColor, "(%s)", randoItemName.c_str());
                    } else if (randoSaveCheck.skipped) {
                        ImGui::SameLine();
                        ImGui::TextColored(itemTextColor, "(Skipped)");
                    }
                    ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 0));
                    ImGui::EndGroup();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::IsItemHovered()
                                                                          ? IM_COL32(255, 255, 0, 128)
                                                                          : IM_COL32(255, 255, 255, 0));
                    if (ImGui::IsItemClicked()) {
                        if (!RANDO_SAVE_CHECKS[randoCheckId].eligible) {
                            if (!RANDO_SAVE_CHECKS[randoCheckId].skipped) {
                                CheckTracker_AddToCheckCount(randoCheckId);
                            } else {
                                CheckTracker_RemoveFromCheckCount(randoCheckId);
                            }
                            RANDO_SAVE_CHECKS[randoCheckId].skipped = !RANDO_SAVE_CHECKS[randoCheckId].skipped;
                        }
                    }
                    ImGui::TableNextColumn();
                }
                ImGui::EndTable();
            }
            ImGui::Unindent(20.0f);
        }
        ImGui::PopStyleColor(2);
        ImGui::PopID();
    }
}

namespace Rando {

namespace CheckTracker {

void LoadFromPreset(const nlohmann::json& info) {
    presetLoaded = true;
    presetPos = { info.at("pos").at("x"), info.at("pos").at("y") };
    presetSize = { info.at("size").at("width"), info.at("size").at("height") };
}

void CheckTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, checkTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, checkTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, checkTrackerBG);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    if (presetLoaded) {
        ImGui::SetNextWindowSize(presetSize);
        ImGui::SetNextWindowPos(presetPos);
        presetLoaded = false;
    } else {
        ImGui::SetNextWindowSize(ImVec2(485.0f, 500.0f), ImGuiCond_FirstUseEver);
    }

    if (Prefs::Trackers::CheckTracker::ShowCollectedChecks && Prefs::Trackers::CheckTracker::SeparateCollectedChecks) {
        CheckTracker_DrawCheckCount();
    }

    if (ImGui::Begin("CheckTracker", nullptr, windowFlags)) {
        checkTrackerBG.w = ImGui::IsWindowDocked() ? 1.0f : Prefs::Trackers::CheckTracker::Opacity.Float();
        ImGui::SetWindowFontScale(checkTrackerScale);

        if (gsworld_getMap() == MAP_91_FILE_SELECT) {
            ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange), "No Rando File Selected...");
        } else {
            if (ImGui::BeginChild("CheckTrackerChild")) {
                DrawCheckTrackerList();
                expandState = expandToggle;
                ImGui::EndChild();
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
}

void SettingsWindow::DrawElement() {
    windowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;
    if (Prefs::Trackers::CheckTracker::Floating) {
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
    }

    UIWidgets::WindowButton("Popout Check Tracker", LighthouseGui::mRandoCheckTrackerWindow,
                            UIWidgets::WindowButtonOptions()
                                .Size(UIWidgets::Sizes::Inline)
                                .Color(UIWidgets::Colors::Green)
                                .OpenLabel("Return Check Tracker")
                                .OpenColor(UIWidgets::Colors::Red));
    if (ImGui::BeginTable("SettingsTable", 2)) {
        ImGui::TableSetupColumn("col1", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("col2", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();

        ImGui::SeparatorText("Check Tracker");
        if (!LighthouseGui::mRandoCheckTrackerWindow->IsVisible()) {
            if (ImGui::BeginChild("EmbeddedCheckTrackerChild")) {
                DrawCheckTrackerList();
                ImGui::EndChild();
            }
        } else {
            ImGui::TextColored(UIWidgets::ColorValues.at(WIDGET_COLOR), "Tracker popped out");
        }

        ImGui::TableNextColumn();
        ImGui::SeparatorText("Window Settings");
        if (ImGui::BeginChild("WindowSettingsChild")) {
            UIWidgets::PrefCheckbox("Only Show Current Level", UIWidgets::CheckboxOptions().Setting(
                                                                   &Prefs::Trackers::CheckTracker::ShowCurrentLevel));
            UIWidgets::PrefCheckbox("Dim Out of Logic Checks",
                                    UIWidgets::CheckboxOptions().Setting(&Prefs::Trackers::CheckTracker::ShowLogic));
            UIWidgets::PrefCheckbox("Hide Completed Worlds", UIWidgets::CheckboxOptions().Setting(
                                                                 &Prefs::Trackers::CheckTracker::HideCompletedWorld));
            UIWidgets::PrefCheckbox("Hide Collected Checks", UIWidgets::CheckboxOptions().Setting(
                                                                 &Prefs::Trackers::CheckTracker::HideCollected));
            UIWidgets::PrefCheckbox("Hide Skipped Checks",
                                    UIWidgets::CheckboxOptions().Setting(&Prefs::Trackers::CheckTracker::HideSkipped));
            UIWidgets::PrefCheckbox(
                "Display Total Collected Checks",
                UIWidgets::CheckboxOptions().Setting(&Prefs::Trackers::CheckTracker::ShowCollectedChecks));
            UIWidgets::PrefCheckbox("Display Total World Checks", UIWidgets::CheckboxOptions().Setting(
                                                                      &Prefs::Trackers::CheckTracker::ShowWorldChecks));

            ImGui::BeginDisabled(!Prefs::Trackers::CheckTracker::ShowCollectedChecks);
            UIWidgets::PrefCheckbox(
                "Separate Total Collected Checks",
                UIWidgets::CheckboxOptions().Setting(&Prefs::Trackers::CheckTracker::SeparateCollectedChecks));
            ImGui::EndDisabled();
            ImGui::BeginDisabled(!Prefs::Trackers::CheckTracker::SeparateCollectedChecks ||
                                 !Prefs::Trackers::CheckTracker::ShowCollectedChecks);
            if (UIWidgets::PrefSlider("Collected Checks Opacity", UIWidgets::SliderOptions()
                                                .Setting(&Prefs::Trackers::CheckTracker::CollectedChecksOpacity)
                                                .Display(UIWidgets::SliderDisplay::Float)
                                                .Prefix("Opacity: ")
                                                .LabelPosition(UIWidgets::LabelPositions::None)
                                                .Color(WIDGET_COLOR))) {
                collectedChecksBG.w = Prefs::Trackers::CheckTracker::CollectedChecksOpacity.Float();
            }

            if (UIWidgets::PrefSlider("Collected Checks Scale", UIWidgets::SliderOptions()
                                                  .Setting(&Prefs::Trackers::CheckTracker::CollectedChecksScale)
                                                  .Display(UIWidgets::SliderDisplay::Float)
                                                  .Prefix("Scale: ")
                                                  .LabelPosition(UIWidgets::LabelPositions::None)
                                                  .Color(WIDGET_COLOR))) {
                collectedChecksScale = Prefs::Trackers::CheckTracker::CollectedChecksScale.Float();
            }

            ImGui::EndDisabled();

            UIWidgets::PrefCheckbox("Toggle Floating Window",
                                    UIWidgets::CheckboxOptions().Setting(&Prefs::Trackers::CheckTracker::Floating));

            if (UIWidgets::Button(
                    "Expand/Collapse All Levels",
                    UIWidgets::ButtonOptions{}.Color(WIDGET_COLOR).Size(ImVec2(ImGui::GetContentRegionAvail().x, 0)))) {
                expandToggle = !expandToggle;
            }

            if (UIWidgets::PrefSlider("Tracker Opacity", UIWidgets::SliderOptions()
                                              .Setting(&Prefs::Trackers::CheckTracker::Opacity)
                                              .Display(UIWidgets::SliderDisplay::Float)
                                              .Prefix("Opacity: ")
                                              .LabelPosition(UIWidgets::LabelPositions::None)
                                              .Color(WIDGET_COLOR))) {
                checkTrackerBG.w = Prefs::Trackers::CheckTracker::Opacity.Float();
            }

            if (UIWidgets::PrefSlider("Tracker Scale", UIWidgets::SliderOptions()
                                               .Setting(&Prefs::Trackers::CheckTracker::Scale)
                                               .Display(UIWidgets::SliderDisplay::Float)
                                               .Prefix("Scale: ")
                                               .LabelPosition(UIWidgets::LabelPositions::None)
                                               .Color(WIDGET_COLOR))) {
                checkTrackerScale = Prefs::Trackers::CheckTracker::Scale.Float();
            }

            int16_t checkColorIndex = 0;
            for (auto& [pref, label] : checkColorList) {
                std::string colorText = label;
                colorText += " Color";
                std::string widgetLabel = "##";
                widgetLabel += std::to_string(checkColorIndex);

                ImGui::PushID(checkColorIndex);
                UIWidgets::PrefColorPicker(widgetLabel.c_str(),
                                           UIWidgets::ColorPickerOptions().Setting(pref).UseAlpha());
                ImGui::SameLine();
                if (UIWidgets::Button(ICON_FA_REFRESH, { .size = ImVec2(32.0f, 32.0f), .color = WIDGET_COLOR })) {
                    pref->Reset();
                }
                ImGui::SameLine();
                ImGui::Text(colorText.c_str());
                ImGui::PopID();
                checkColorIndex++;
            }
            ImGui::EndChild();
        }
        ImGui::EndTable();
    }
}

void Init() {
    checkTrackerBG = { 0, 0, 0, Prefs::Trackers::CheckTracker::Opacity.Float() };
    collectedChecksBG = { 0, 0, 0, Prefs::Trackers::CheckTracker::CollectedChecksOpacity.Float() };
    checkTrackerScale = Prefs::Trackers::CheckTracker::Scale.Float();
    collectedChecksScale = Prefs::Trackers::CheckTracker::CollectedChecksScale.Float();
}

} // namespace CheckTracker
} // namespace Rando

void RegisterCheckTracker() {
    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) { Rando::CheckTracker::Init(); });

    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, IS_RANDO, [](IEvent* event) {
        CheckTracker_CreateCheckList();
        CheckTracker_InitiateTotals();
    });
}

static RegisterShipInitFunc initFunc(RegisterCheckTracker, { "IS_RANDO" });
