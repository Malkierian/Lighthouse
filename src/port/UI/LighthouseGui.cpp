#include "LighthouseGui.hpp"

#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_internal.h>
#include "UIWidgets.hpp"
#include "src/port/DevTools/EventDebugger.h"
#include "src/port/DevTools/OcclusionDebug.h"

#ifdef __APPLE__
#include <fast/backends/gfx_metal.h>
#endif

#ifdef __SWITCH__
#include <port/switch/SwitchImpl.h>
#endif

#include "port/Prefs/Pref.h"

// #include "Enhancements/Trackers/ItemTracker/ItemTracker.h"
// #include "Enhancements/Trackers/ItemTracker/ItemTrackerSettings.h"
#include "port/Enhancements/Trackers/DisplayOverlay.h"
#include "port/Enhancements/Trackers/WorldTracker/WorldTracker.h"
// #include "Enhancements/Trackers//TimeSplits/Timesplits.h"
// #include "Enhancements/Trackers/TimeSplits/TimesplitsSettings.h"
#include "port/Rando/CheckTracker/CheckTracker.h"

#include "Notification.h"
#include "port/Controller/Mapper.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Backports/EggAim.h"
#include "LighthouseMenu.h"
#include "LighthouseInputEditorWindow.h"
#include "LighthouseModMenuWindow.h"
// #include "DeveloperTools/HookDebugger.h"
#include "DeveloperTools/SaveEditor.h"
#include "DeveloperTools/GameplayTools.h"
#include "port/Prefs/Sections/SettingsPrefs.h"
// #include "DeveloperTools/ActorViewer.h"
// #include "DeveloperTools/CollisionViewer.h"
// #include "DeveloperTools/EventLog.h"
// #include "DeveloperTools/DLViewer.h"
// #include "DeveloperTools/MessageViewer.h"

namespace LighthouseGui {
// MARK: - Delegates

std::shared_ptr<Ship::GuiWindow> mConsoleWindow;
std::shared_ptr<Ship::GuiWindow> mStatsWindow;
std::shared_ptr<Ship::GuiWindow> mGfxDebuggerWindow;
std::shared_ptr<LighthouseInputEditorWindow> mInputEditorWindow;
std::shared_ptr<Mapper::MapperWindow> mGamepadMapperWindow;
std::shared_ptr<LighthouseModMenuWindow> mModMenuWindow;
std::shared_ptr<LighthouseRomhackMenuWindow> mRomhackMenuWindow;

// std::shared_ptr<HookDebuggerWindow> mHookDebuggerWindow;
std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
std::shared_ptr<GameplayToolsWindow> mGameplayToolsWindow;
// std::shared_ptr<HudEditorWindow> mHudEditorWindow;
// std::shared_ptr<CosmeticEditorWindow> mCosmeticEditorWindow;
// std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
// std::shared_ptr<CollisionViewerWindow> mCollisionViewerWindow;
// std::shared_ptr<EventLogWindow> mEventLogWindow;
// std::shared_ptr<DLViewerWindow> mDLViewerWindow;
// std::shared_ptr<MessageViewerWindow> mMessageViewerWindow;
// std::shared_ptr<AudioEditor> mAudioEditorWindow;
std::shared_ptr<LighthouseMenu> mLighthouseMenu;
std::shared_ptr<Notification::Window> mNotificationWindow;
std::shared_ptr<Rando::CheckTracker::CheckTrackerWindow> mRandoCheckTrackerWindow;
std::shared_ptr<Rando::CheckTracker::SettingsWindow> mRandoCheckTrackerSettingsWindow;
std::shared_ptr<WorldTracker::WorldTrackerWindow> mWorldTrackerWindow;
std::shared_ptr<WorldTracker::SettingsWindow> mWorldTrackerSettingsWindow;
// std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
// std::shared_ptr<ItemTrackerSettingsWindow> mItemTrackerSettingsWindow;
std::shared_ptr<DisplayOverlayWindow> mDisplayOverlayWindow;
// std::shared_ptr<TimesplitsWindow> mTimesplitsWindow;
// std::shared_ptr<TimesplitsSettingsWindow> mTimesplitsSettingsWindow;
std::shared_ptr<InputViewer> mInputViewer;
std::shared_ptr<InputViewerSettingsWindow> mInputViewerSettings;
std::shared_ptr<EggAimCrosshairWindow> mEggAimCrosshair;
std::shared_ptr<LighthouseModalWindow> mModalWindow;
std::shared_ptr<EventDebuggerWindow> mEventDebuggerWindow;
std::shared_ptr<OcclusionDebugWindow> mOcclusionDebugWindow;
std::shared_ptr<AnchorRoomWindow> mAnchorRoomWindow;

namespace WindowPrefs = Prefs::Settings::Windows;

void BindVisibility(const std::shared_ptr<Ship::GuiWindow>& window, Prefs::Bool& pref) {
    if (window == nullptr) {
        return;
    }
    std::weak_ptr<Ship::GuiWindow> weakWindow = window;
    pref.SetOnChange([weakWindow, &pref](Prefs::Base&) {
        const auto shared = weakWindow.lock();
        if (shared != nullptr && shared->IsVisible() != pref.Get()) {
            shared->ToggleVisibility();
        }
    });
}

UIWidgets::Colors GetMenuThemeColor() {
    return mLighthouseMenu->GetMenuThemeColor();
}

void SetupMenu() {
    auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();
    mLighthouseMenu = std::make_shared<LighthouseGui::LighthouseMenu>(WindowPrefs::Menu.CVar(), "Port Menu");
    gui->SetMenu(mLighthouseMenu);
    BindVisibility(mLighthouseMenu, WindowPrefs::Menu);

    mModalWindow = std::make_shared<LighthouseModalWindow>("", true, "Modal Window");
    gui->AddGuiWindow(mModalWindow);
}

void SetupGuiElements() {
    auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();

    auto& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(4.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.Colors[ImGuiCol_MenuBarBg] = UIWidgets::ColorValues.at(UIWidgets::Colors::DarkGray);

    mStatsWindow = gui->GetGuiWindow("Stats");
    if (mStatsWindow == nullptr) {
        SPDLOG_ERROR("Could not find stats window");
    }
    BindVisibility(mStatsWindow, WindowPrefs::Stats);

    mConsoleWindow = gui->GetGuiWindow("Console");
    if (mConsoleWindow == nullptr) {
        SPDLOG_ERROR("Could not find console window");
    }
    BindVisibility(mConsoleWindow, WindowPrefs::Console);

    // mGfxDebuggerWindow = gui->GetGuiWindow("GfxDebuggerWindow");
    // if (mGfxDebuggerWindow == nullptr) {
    //     SPDLOG_ERROR("Could not find input GfxDebuggerWindow");
    // }

    mInputEditorWindow = std::make_shared<LighthouseInputEditorWindow>(WindowPrefs::ControllerConfiguration.CVar(),
                                                                       "Configure Controller");
    gui->AddGuiWindow(mInputEditorWindow);
    BindVisibility(mInputEditorWindow, WindowPrefs::ControllerConfiguration);

    mGamepadMapperWindow =
        std::make_shared<Mapper::MapperWindow>(WindowPrefs::GamepadMapper.CVar(), "Gamepad Mapper", ImVec2(1280, 820));
    gui->AddGuiWindow(mGamepadMapperWindow);
    BindVisibility(mGamepadMapperWindow, WindowPrefs::GamepadMapper);

    mModMenuWindow = std::make_shared<LighthouseModMenuWindow>(WindowPrefs::ModMenu.CVar(), "Mod Menu");
    gui->AddGuiWindow(mModMenuWindow);
    BindVisibility(mModMenuWindow, WindowPrefs::ModMenu);

    mRomhackMenuWindow = std::make_shared<LighthouseRomhackMenuWindow>(WindowPrefs::RomhackMenu.CVar(), "Romhack Menu");
    gui->AddGuiWindow(mRomhackMenuWindow);
    BindVisibility(mRomhackMenuWindow, WindowPrefs::RomhackMenu);

    // mHookDebuggerWindow =
    //     std::make_shared<HookDebuggerWindow>({CVAR_WINDOW("HookDebugger")}, "Hook Debugger", ImVec2(480, 600));
    // gui->AddGuiWindow(mHookDebuggerWindow);

    mSaveEditorWindow =
        std::make_shared<SaveEditorWindow>(WindowPrefs::SaveEditor.CVar(), "Save Editor", ImVec2(480, 600));
    gui->AddGuiWindow(mSaveEditorWindow);
    BindVisibility(mSaveEditorWindow, WindowPrefs::SaveEditor);

    mGameplayToolsWindow =
        std::make_shared<GameplayToolsWindow>(WindowPrefs::GameplayTools.CVar(), "Gameplay Tools", ImVec2(480, 600));
    gui->AddGuiWindow(mGameplayToolsWindow);
    BindVisibility(mGameplayToolsWindow, WindowPrefs::GameplayTools);

    // mHudEditorWindow = std::make_shared<HudEditorWindow>(CVAR_WINDOW("HudEditor"), "HUD Editor", ImVec2(480, 600));
    // gui->AddGuiWindow(mHudEditorWindow);

    // mCosmeticEditorWindow =
    //     std::make_shared<CosmeticEditorWindow>(CVAR_WINDOW("CosmeticEditor"), "Cosmetic Editor", ImVec2(480, 600));
    // gui->AddGuiWindow(mCosmeticEditorWindow);

    // mActorViewerWindow = std::make_shared<ActorViewerWindow>(CVAR_WINDOW("ActorViewer"), "Actor Viewer", ImVec2(520,
    // 600)); gui->AddGuiWindow(mActorViewerWindow);

    // mCollisionViewerWindow =
    //     std::make_shared<CollisionViewerWindow>(CVAR_WINDOW("CollisionViewer"), "Collision Viewer", ImVec2(390,
    //     475));
    // gui->AddGuiWindow(mCollisionViewerWindow);

    // mEventLogWindow = std::make_shared<EventLogWindow>(CVAR_WINDOW("EventLog"), "Event Log", ImVec2(520, 600));
    // gui->AddGuiWindow(mEventLogWindow);

    // mDLViewerWindow = std::make_shared<DLViewerWindow>(CVAR_WINDOW("DLViewer"), "DL Viewer", ImVec2(520, 600));
    // gui->AddGuiWindow(mDLViewerWindow);
    // mMessageViewerWindow =
    //     std::make_shared<MessageViewerWindow>(CVAR_WINDOW("MessageViewer"), "Message Viewer", ImVec2(520, 600));
    // gui->AddGuiWindow(mMessageViewerWindow);

    // mAudioEditorWindow = std::make_shared<AudioEditor>(CVAR_WINDOW("AudioEditor"), "Audio Editor", ImVec2(520, 600));
    // gui->AddGuiWindow(mAudioEditorWindow);

    // mItemTrackerWindow = std::make_shared<ItemTrackerWindow>(CVAR_WINDOW("ItemTracker"), "Item Tracker");
    // gui->AddGuiWindow(mItemTrackerWindow);

    // mItemTrackerSettingsWindow = std::make_shared<ItemTrackerSettingsWindow>(CVAR_WINDOW("ItemTrackerSettings"),
    //                                                                          "Item Tracker Settings", ImVec2(800,
    //                                                                          400));
    // gui->AddGuiWindow(mItemTrackerSettingsWindow);

    mDisplayOverlayWindow = std::make_shared<DisplayOverlayWindow>("", "Display Overlay");
    gui->AddGuiWindow(mDisplayOverlayWindow);

    // mTimesplitsWindow = std::make_shared<TimesplitsWindow>(CVAR_WINDOW("Timesplits"), "Time Splits Window");
    // gui->AddGuiWindow(mTimesplitsWindow);

    // mTimesplitsSettingsWindow = std::make_shared<TimesplitsSettingsWindow>(
    //     CVAR_WINDOW("Timesplits.Settings"), "Time Splits Settings Window", ImVec2(567, 97));
    // gui->AddGuiWindow(mTimesplitsSettingsWindow);

    mNotificationWindow = std::make_shared<Notification::Window>("", true, "Notifications Window");
    gui->AddGuiWindow(mNotificationWindow);

    mRandoCheckTrackerWindow = std::make_shared<Rando::CheckTracker::CheckTrackerWindow>(
        WindowPrefs::CheckTracker.CVar(), "Check Tracker", ImVec2(375, 460));
    gui->AddGuiWindow(mRandoCheckTrackerWindow);
    BindVisibility(mRandoCheckTrackerWindow, WindowPrefs::CheckTracker);

    mRandoCheckTrackerSettingsWindow = std::make_shared<Rando::CheckTracker::SettingsWindow>(
        WindowPrefs::CheckTrackerSettings.CVar(), "Check Tracker Settings");
    gui->AddGuiWindow(mRandoCheckTrackerSettingsWindow);
    BindVisibility(mRandoCheckTrackerSettingsWindow, WindowPrefs::CheckTrackerSettings);

    mWorldTrackerWindow = std::make_shared<WorldTracker::WorldTrackerWindow>(WindowPrefs::WorldTracker.CVar(),
                                                                             "World Tracker", ImVec2(375, 460));
    gui->AddGuiWindow(mWorldTrackerWindow);
    BindVisibility(mWorldTrackerWindow, WindowPrefs::WorldTracker);

    mWorldTrackerSettingsWindow = std::make_shared<WorldTracker::SettingsWindow>(
        WindowPrefs::WorldTrackerSettings.CVar(), "World Tracker Settings");
    gui->AddGuiWindow(mWorldTrackerSettingsWindow);
    BindVisibility(mWorldTrackerSettingsWindow, WindowPrefs::WorldTrackerSettings);

    mEggAimCrosshair = std::make_shared<EggAimCrosshairWindow>("", true, "Egg Aim Crosshair");
    gui->AddGuiWindow(mEggAimCrosshair);

    mInputViewer = std::make_shared<InputViewer>(WindowPrefs::InputViewer.CVar(), "Input Viewer");
    gui->AddGuiWindow(mInputViewer);
    BindVisibility(mInputViewer, WindowPrefs::InputViewer);

    mInputViewerSettings = std::make_shared<InputViewerSettingsWindow>(WindowPrefs::InputViewerSettings.CVar(),
                                                                       "Input Viewer Settings", ImVec2(500, 525));
    gui->AddGuiWindow(mInputViewerSettings);
    BindVisibility(mInputViewerSettings, WindowPrefs::InputViewerSettings);

    mEventDebuggerWindow = std::make_shared<EventDebuggerWindow>(WindowPrefs::EventDebugger.CVar(), "Event Debugger");
    gui->AddGuiWindow(mEventDebuggerWindow);
    BindVisibility(mEventDebuggerWindow, WindowPrefs::EventDebugger);

    mOcclusionDebugWindow =
        std::make_shared<OcclusionDebugWindow>(WindowPrefs::OcclusionDebug.CVar(), "Occlusion Debugger");
    gui->AddGuiWindow(mOcclusionDebugWindow);
    BindVisibility(mOcclusionDebugWindow, WindowPrefs::OcclusionDebug);

    mAnchorRoomWindow = std::make_shared<AnchorRoomWindow>(WindowPrefs::AnchorRoom.CVar(), "Anchor Room");
    gui->AddGuiWindow(mAnchorRoomWindow);
    BindVisibility(mAnchorRoomWindow, WindowPrefs::AnchorRoom);
}

void Destroy() {
    auto gui = Ship::Context::GetRawInstance()->GetWindow()->GetGui();

    gui->RemoveAllGuiWindows();
    mLighthouseMenu = nullptr;
    mModalWindow = nullptr;
    mStatsWindow = nullptr;
    mConsoleWindow = nullptr;
    mGfxDebuggerWindow = nullptr;
    mInputEditorWindow = nullptr;
    mGamepadMapperWindow = nullptr;
    mModMenuWindow = nullptr;
    mRomhackMenuWindow = nullptr;
    // mCollisionViewerWindow = nullptr;
    // mEventLogWindow = nullptr;
    mNotificationWindow = nullptr;
    mRandoCheckTrackerWindow = nullptr;
    mRandoCheckTrackerSettingsWindow = nullptr;
    mWorldTrackerWindow = nullptr;
    mWorldTrackerSettingsWindow = nullptr;

    // mHookDebuggerWindow = nullptr;
    mSaveEditorWindow = nullptr;
    mGameplayToolsWindow = nullptr;
    // mHudEditorWindow = nullptr;
    // mCosmeticEditorWindow = nullptr;
    // mActorViewerWindow = nullptr;
    // mDLViewerWindow = nullptr;
    // mMessageViewerWindow = nullptr;
    // mAudioEditorWindow = nullptr;
    // mItemTrackerWindow = nullptr;
    // mItemTrackerSettingsWindow = nullptr;
    mDisplayOverlayWindow = nullptr;
    mInputViewer = nullptr;
    mInputViewerSettings = nullptr;
    mEggAimCrosshair = nullptr;
    mEventDebuggerWindow = nullptr;
    mOcclusionDebugWindow = nullptr;
    mAnchorRoomWindow = nullptr;
}

void RegisterPopup(std::string title, std::string message, std::string button1, std::string button2,
                   std::function<void()> button1callback, std::function<void()> button2callback) {
    mModalWindow->RegisterPopup(title, message, button1, button2, button1callback, button2callback);
}

size_t PopupsQueued() {
    return mModalWindow->PopupsQueued();
}

} // namespace LighthouseGui

// Prefs
namespace Prefs::Settings {

namespace Windows {

static Prefs::Options<bool> Synced(const char* cvar) {
    return Prefs::Options<bool>().CVar(cvar).SyncCVar();
}

std::string confPath = "Windows.";
Prefs::Bool Menu{ PrefSection::SECTION_SETTINGS, confPath + "Menu", false, Synced(CVAR_WINDOW("Menu")) };
Prefs::Bool ControllerConfiguration{ PrefSection::SECTION_SETTINGS, confPath + "ControllerConfiguration", false,
                                     Synced(CVAR_WINDOW("ControllerConfiguration")) };
Prefs::Bool GamepadMapper{ PrefSection::SECTION_SETTINGS, confPath + "GamepadMapper", false,
                           Synced(CVAR_WINDOW("GamepadMapper")) };
Prefs::Bool ModMenu{ PrefSection::SECTION_SETTINGS, confPath + "ModMenu", false, Synced(CVAR_WINDOW("ModMenu")) };
Prefs::Bool RomhackMenu{ PrefSection::SECTION_SETTINGS, confPath + "RomhackMenu", false,
                         Synced(CVAR_WINDOW("RomhackMenu")) };
Prefs::Bool SaveEditor{ PrefSection::SECTION_SETTINGS, confPath + "SaveEditor", false,
                        Synced(CVAR_WINDOW("SaveEditor")) };
Prefs::Bool GameplayTools{ PrefSection::SECTION_SETTINGS, confPath + "GameplayTools", false,
                           Synced(CVAR_WINDOW("GameplayTools")) };
Prefs::Bool CheckTracker{ PrefSection::SECTION_SETTINGS, confPath + "CheckTracker", false,
                          Synced(CVAR_WINDOW("CheckTracker")) };
Prefs::Bool CheckTrackerSettings{ PrefSection::SECTION_SETTINGS, confPath + "CheckTrackerSettings", false,
                                  Synced(CVAR_WINDOW("CheckTrackerSettings")) };
Prefs::Bool WorldTracker{ PrefSection::SECTION_SETTINGS, confPath + "WorldTracker", false,
                          Synced(CVAR_WINDOW("WorldTracker")) };
Prefs::Bool WorldTrackerSettings{ PrefSection::SECTION_SETTINGS, confPath + "WorldTrackerSettings", false,
                                  Synced(CVAR_WINDOW("WorldTrackerSettings")) };
Prefs::Bool InputViewer{ PrefSection::SECTION_SETTINGS, confPath + "InputViewer", false,
                         Synced(CVAR_WINDOW("InputViewer")) };
Prefs::Bool InputViewerSettings{ PrefSection::SECTION_SETTINGS, confPath + "InputViewerSettings", false,
                                 Synced(CVAR_WINDOW("InputViewerSettings")) };
Prefs::Bool EventDebugger{ PrefSection::SECTION_SETTINGS, confPath + "EventDebugger", false,
                           Synced(CVAR_WINDOW("EventDebugger")) };
Prefs::Bool OcclusionDebug{ PrefSection::SECTION_SETTINGS, confPath + "OcclusionDebug", false,
                            Synced(CVAR_WINDOW("OcclusionDebug")) };
Prefs::Bool AnchorRoom{ PrefSection::SECTION_SETTINGS, confPath + "AnchorRoom", false,
                        Synced(CVAR_WINDOW("AnchorRoom")) };
// Match CVAR_STATS_WINDOW_OPEN and CVAR_CONSOLE_WINDOW_OPEN via lus-cvars.cmake.
Prefs::Bool Stats{ PrefSection::SECTION_SETTINGS, confPath + "Stats", false, Synced(CVAR_WINDOW("Stats")) };
Prefs::Bool Console{ PrefSection::SECTION_SETTINGS, confPath + "Console", false, Synced(CVAR_WINDOW("Console")) };

} // namespace Windows

} // namespace Prefs::Settings
