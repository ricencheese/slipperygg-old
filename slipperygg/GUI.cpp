﻿//window size no more than 1280x720, please :l
#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#ifdef _WIN32
#include <ShlObj.h>
#include <Windows.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include "imguiCustom.h"

#include "GUI.h"
#include "Config.h"
#include "ConfigStructs.h"
#include "Hacks/Misc.h"
#include "InventoryChanger/InventoryChanger.h"
#include "Helpers.h"
#include "Interfaces.h"
#include "../SDK/Engine.h"
#include "SDK/InputSystem.h"
#include "SDK/LocalPlayer.h"
#include "Hacks/Visuals.h"
#include "Hacks/Glow.h"
#include "Hacks/AntiAim.h"
#include "Hacks/Backtrack.h"
#include "Hacks/Sound.h"
#include "Hacks/StreamProofESP.h"
#include "Hooks.h"

constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
| ImGuiWindowFlags_NoScrollbar;

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept
{
    auto file = Helpers::loadBinaryFile(path);
    if (!Helpers::decodeVFONT(file))
        return nullptr;

    ImFontConfig cfg;
    cfg.FontData = file.data();
    cfg.FontDataSize = file.size();
    cfg.FontDataOwnedByAtlas = false;
    cfg.MergeMode = merge;
    cfg.GlyphRanges = glyphRanges;
    cfg.SizePixels = size;

    return ImGui::GetIO().Fonts->AddFont(&cfg);
}

GUI::GUI() noexcept
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScrollbarSize = 9.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImFontConfig cfg;
    cfg.SizePixels = 15.0f;

    ImFontConfig cfgbackground;
    cfgbackground.SizePixels = 70.0f;

    ImFontConfig cfgIcons;
    cfgIcons.SizePixels = 47.0f;

#ifdef _WIN32           //fontssss
    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);
        fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.normal15px)
            io.Fonts->AddFontDefault(&cfg);

        cfg.MergeMode = true;
        static constexpr ImWchar symbol[]{
            0x2605, 0x2605, // ★
            0
        };
        io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
        cfg.MergeMode = false;
    }
    if (PWSTR pathToRoaming; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathToRoaming))) {
        const std::filesystem::path path{ pathToRoaming };
        CoTaskMemFree(pathToRoaming);
        fonts.backgroundCubes = io.Fonts->AddFontFromFileTTF((path / "slippery/dependencies/background.ttf").string().c_str(), 70.0f, &cfgbackground, Helpers::getFontGlyphRanges());
        fonts.icons = io.Fonts->AddFontFromFileTTF((path / "slippery/dependencies/icons.ttf").string().c_str(), 47.0f, &cfgbackground, Helpers::getFontGlyphRanges());
}
#else
    fonts.normal15px = addFontFromVFONT("csgo/panorama/fonts/notosans-regular.vfont", 15.0f, Helpers::getFontGlyphRanges(), false);
#endif
    if (!fonts.normal15px)
        io.Fonts->AddFontDefault(&cfg);
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 17.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
}

void GUI::render() noexcept
{
    renderGuiStyle3();
}

void GUI::updateColors() const noexcept
{
    switch (config->style.menuColors) {
    case 0: ImGui::StyleColorsDark(); break;
    case 1: ImGui::StyleColorsLight(); break;
    case 2: ImGui::StyleColorsClassic(); break;
    }
}

void GUI::handleToggle() noexcept
{
    if (Misc::isMenuKeyPressed()) {
        open = !open;
        if (!open)
            interfaces->inputSystem->resetInputState();
#ifndef _WIN32
        ImGui::GetIO().MouseDrawCursor = gui->open;
#endif
    }
}

                                                //TODO: fix this
//static ImTextureID getItemIconTexture(std::string_view iconpath) noexcept           
//{
//    if (iconpath.empty())
//        return 0;
//
//    auto& icon = iconTextures[std::string{ iconpath }];
//    if (!icon.texture.get()) {
//        static int frameCount = 0;
//        static float timeSpentThisFrame = 0.0f;
//        static int loadedThisFrame = 0;
//
//        if (frameCount != ImGui::GetFrameCount()) {
//            frameCount = ImGui::GetFrameCount();
//            timeSpentThisFrame = 0.0f;
//            // memory->debugMsg("LOADED %d ICONS\n", loadedThisFrame);
//            loadedThisFrame = 0;
//        }
//
//        if (timeSpentThisFrame > 0.01f)
//            return 0;
//
//        ++loadedThisFrame;
//
//        const auto start = std::chrono::high_resolution_clock::now();
//
//        auto handle = interfaces->baseFileSystem->open(("resource/flash/" + std::string{ iconpath } + (iconpath.find("status_icons") != std::string_view::npos ? "" : "_large") + ".png").c_str(), "r", "GAME");
//        if (!handle)
//            handle = interfaces->baseFileSystem->open(("resource/flash/" + std::string{ iconpath } + ".png").c_str(), "r", "GAME");
//
//        assert(handle);
//        if (handle) {
//            if (const auto size = interfaces->baseFileSystem->size(handle); size > 0) {
//                const auto buffer = std::make_unique<std::uint8_t[]>(size);
//                if (interfaces->baseFileSystem->read(buffer.get(), size, handle) > 0) {
//                    int width, height;
//                    stbi_set_flip_vertically_on_load_thread(false);
//
//                    if (const auto data = stbi_load_from_memory((const stbi_uc*)buffer.get(), size, &width, &height, nullptr, STBI_rgb_alpha)) {
//                        icon.texture.init(width, height, data);
//                        stbi_image_free(data);
//                    }
//                    else {
//                        assert(false);
//                    }
//                }
//            }
//            interfaces->baseFileSystem->close(handle);
//        }
//
//        const auto end = std::chrono::high_resolution_clock::now();
//        timeSpentThisFrame += std::chrono::duration<float>(end - start).count();
//    }
//    icon.lastReferencedFrame = ImGui::GetFrameCount();
//    return icon.texture.get();
//}

static void menuBarItem(const char* name, bool& enabled) noexcept
{
    if (ImGui::MenuItem(name)) {
        enabled = true;
        ImGui::SetWindowFocus(name);
        ImGui::SetWindowPos(name, { 100.0f, 100.0f });
    }
}


void GUI::renderMenuBar() noexcept          //THIS IS OLD TOP MENU BAR STYLE MENU!!!!!!! SCROLL DOWN FOR THE NEW STYLE MENU RETARD
{
    if (ImGui::BeginMainMenuBar()) {
        menuBarItem("Aimbot", window.aimbot);
        menuBarItem("Home", window.home);
        //AntiAim::menuBarItem(); //antiaim in a legit cheat?? it's useless and dangerous (animfix doesn't show you aa'ing)
        menuBarItem("Triggerbot", window.triggerbot);
        Backtrack::menuBarItem();
        Glow::menuBarItem();
        menuBarItem("Chams", window.chams);
        StreamProofESP::menuBarItem();
        Visuals::menuBarItem();
        InventoryChanger::menuBarItem();
        Sound::menuBarItem();
        menuBarItem("Style", window.style);//probably going to replace it with dark/light mode as soon as I code them :waaaa:
        Misc::menuBarItem();
        menuBarItem("Config", window.config);
        ImGui::EndMainMenuBar();   
    }
}
int sidebarSpeed[]{ 30 };
void GUI::renderHomeWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (window3.curWindow!=0)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Home", &window3.home, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    ImGui::SetCursorPos(ImVec2(19, 8));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5, 0.5, 0.5, 1));
    ImGui::BeginChild("HomeContent1", ImVec2(258, 416), true);
    ImGui::SliderInt("sidebar speed", sidebarSpeed, 1, 200);
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Dark theme\0Light theme\0"))
        updateColors();
    ImGui::Combo("Menu background", &window.backgroundGraphics, "Cubes\0Dots\0");
    ImGui::EndChild();
    ImGui::SetCursorPos(ImVec2(286, 8));
    ImGui::BeginChild("HomeContent2", ImVec2(258, 416),true);
    ImGui::EndChild();
    ImGui::SetCursorPos(ImVec2(554, 8));
    ImGui::BeginChild("HomeContent3", ImVec2(258, 416),true);
    ImGui::EndChild();
    ImGui::PopStyleColor();
    if (!contentOnly)
        ImGui::End();
}

void GUI::beginHighlight(ImVec4 col) noexcept {
    ImGui::PushStyleColor(ImGuiCol_Button, col);
    window3.isHighlighted = true;
};
void GUI::endHighlight() noexcept {
    if(window3.isHighlighted)
    ImGui::PopStyleColor();
    window3.isHighlighted = false;
}
void GUI::renderVisualsWindow(bool contentOnly) noexcept {
    if (!contentOnly) {
        if (window3.curWindow != 2)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("Visual", &window3.visuals, windowFlags);
    };
    if (window3.visualsSub == 0)
        beginHighlight(ImVec4(0.15,0.15,0.15,1));
    if (ImGui::Button("Visuаls", ImVec2(258.f, 25.f))) {
        window3.visualsSub = 0;
    }
    endHighlight();
    ImGui::SameLine();
    if (window3.visualsSub == 1)
        beginHighlight(ImVec4(0.15, 0.15, 0.15, 1));
    if (ImGui::Button("Chams", ImVec2(258.f, 25.f)))
        window3.visualsSub = 1;
    endHighlight();
    ImGui::SameLine();
    if (window3.visualsSub == 2)
        beginHighlight(ImVec4(0.15, 0.15, 0.15, 1));
    if (ImGui::Button("ESP", ImVec2(258.f, 25.f)))
        window3.visualsSub = 2;
    endHighlight();

    ImGui::Separator();

    switch (window3.visualsSub) {
    case 0: Visuals::drawGUI(true); break;
    case 1: renderChamsWindow(true); break;
    case 2: StreamProofESP::drawGUI(true); break;
    };
}

void GUI::renderAimAssistance(bool contentOnly) noexcept {
    if (!contentOnly) {
        if (window3.curWindow != 2)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("Visual", &window3.visuals, windowFlags);
    };
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 1));
    ImGui::SetCursorPos(ImVec2(15, 8));
    ImGui::BeginChild("AimbotSubwindow", ImVec2(395, 424), true);
    renderAimbotWindow(true);
    ImGui::EndChild();
    ImGui::SetCursorPos(ImVec2(419, 8));
    ImGui::BeginChild("TriggerbotSubwindow", ImVec2(395, 424), true);
    renderTriggerbotWindow(true);
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void GUI::renderAimbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (window3.curWindow != 1)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("Aimbot", &window3.aimAssist, windowFlags);
    }
    ImGui::Checkbox("On key", &config->aimbotOnKey);
    ImGui::SameLine();
    ImGui::PushID("Aimbot Key");
    ImGui::hotkey("", config->aimbotKey);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushItemWidth(70.0f);
    ImGui::Combo("", &config->aimbotKeyMode, "Hold\0Toggle\0");
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::Separator();
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);

    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->aimbot[currentWeapon].enabled);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 220.0f);
    ImGui::Checkbox("Aimlock", &config->aimbot[currentWeapon].aimlock);
    ImGui::Checkbox("Silent", &config->aimbot[currentWeapon].silent);
    ImGui::Checkbox("Friendly fire", &config->aimbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Visible only", &config->aimbot[currentWeapon].visibleOnly);
    ImGui::Checkbox("Scoped only", &config->aimbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->aimbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->aimbot[currentWeapon].ignoreSmoke);
    ImGui::Checkbox("Auto shot", &config->aimbot[currentWeapon].autoShot);
    ImGui::Checkbox("Auto scope", &config->aimbot[currentWeapon].autoScope);
    //ImGui::Checkbox("Auto stop", &config->aimbot[currentWeapon].autoStop);
    ImGui::Combo("Bone", &config->aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(60.0f);
    ImGui::SliderFloat("Fov", &config->aimbot[currentWeapon].fov, 0.0f, 255.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Smooth", &config->aimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Aim hitchance", &config->aimbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Shot hitchance", &config->aimbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.2f");
    ImGui::InputInt("Min damage", &config->aimbot[currentWeapon].minDamage);
    config->aimbot[currentWeapon].minDamage = std::clamp(config->aimbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->aimbot[currentWeapon].killshot);
    ImGui::Checkbox("Between shots", &config->aimbot[currentWeapon].betweenShots);
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Text("Backtrack");
    Backtrack::drawGUI(true);
    //Misc::drawMiscAimbot();
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderTriggerbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.triggerbot)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Triggerbot", &window.triggerbot, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0Zeus x27\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);
    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 5:
        currentWeapon = 39;
        ImGui::NewLine();
        break;

    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->triggerbot[currentWeapon].enabled);
    ImGui::Separator();
    ImGui::hotkey("Hold Key", config->triggerbotHoldKey);
    ImGui::Checkbox("Friendly fire", &config->triggerbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Scoped only", &config->triggerbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->triggerbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->triggerbot[currentWeapon].ignoreSmoke);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::Combo("Hitgroup", &config->triggerbot[currentWeapon].hitgroup, "All\0Head\0Chest\0Stomach\0Left arm\0Right arm\0Left leg\0Right leg\0");
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Shot delay", &config->triggerbot[currentWeapon].shotDelay, 0, 250, "%d ms");
    ImGui::InputInt("Min damage", &config->triggerbot[currentWeapon].minDamage);
    config->triggerbot[currentWeapon].minDamage = std::clamp(config->triggerbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->triggerbot[currentWeapon].killshot);
    ImGui::SliderFloat("Burst Time", &config->triggerbot[currentWeapon].burstTime, 0.0f, 0.5f, "%.3f s");

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderChamsWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.chams)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Chams", &window.chams, windowFlags);
    }

    ImGui::hotkey("Toggle Key", config->chamsToggleKey, 80.0f);
    ImGui::hotkey("Hold Key", config->chamsHoldKey, 80.0f);
    ImGui::Separator();

    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);

    static int material = 1;

    if (ImGui::Combo("", &currentCategory, "Allies\0Enemies\0Planting\0Defusing\0Local player\0Weapons\0Hands\0Backtrack\0Sleeves\0"))
        material = 1;

    ImGui::PopID();

    ImGui::SameLine();

    if (material <= 1)
        ImGuiCustom::arrowButtonDisabled("##left", ImGuiDir_Left);
    else if (ImGui::ArrowButton("##left", ImGuiDir_Left))
        --material;

    ImGui::SameLine();
    ImGui::Text("%d", material);

    constexpr std::array categories{ "Allies", "Enemies", "Planting", "Defusing", "Local player", "Weapons", "Hands", "Backtrack", "Sleeves" };

    ImGui::SameLine();

    if (material >= int(config->chams[categories[currentCategory]].materials.size()))
        ImGuiCustom::arrowButtonDisabled("##right", ImGuiDir_Right);
    else if (ImGui::ArrowButton("##right", ImGuiDir_Right))
        ++material;

    ImGui::SameLine();

    auto& chams{ config->chams[categories[currentCategory]].materials[material - 1] };

    ImGui::Checkbox("Enabled", &chams.enabled);
    ImGui::Separator();
    ImGui::Checkbox("Health based", &chams.healthBased);
    ImGui::Checkbox("Blinking", &chams.blinking);
    ImGui::Combo("Material", &chams.material, "Normal\0Flat\0Animated\0Platinum\0Glass\0Chrome\0Crystal\0Silver\0Gold\0Plastic\0Glow\0Pearlescent\0Metallic\0");
    ImGui::Checkbox("Wireframe", &chams.wireframe);
    ImGui::Checkbox("Cover", &chams.cover);
    ImGui::Checkbox("Ignore-Z", &chams.ignorez);
    ImGuiCustom::colorPicker("Color", chams);

    if (!contentOnly) {
        ImGui::End();
    }
}

void GUI::renderStyleWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.style)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Style", &window.style, windowFlags);
    }

    ImGui::PushItemWidth(150.0f);
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Dark\0Light\0Classic\0Custom\0"))
        updateColors();
    ImGui::PopItemWidth();

    if (config->style.menuColors == 3) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));

            ImGuiCustom::colorPicker(ImGui::GetStyleColorName(i), (float*)&style.Colors[i], &style.Colors[i].w);
        }
    }

    if (!contentOnly)
        ImGui::End();
}

ImVec2 slipperyMenuPos{ ImVec2(0,0) };
void GUI::renderGuiStyle3() noexcept {
    int w, h;
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    static ImVec4 col = ImVec4(0.574f, 0.574f, 0.785f, 1.0f);
    const ImU32 col32 = ImColor(col);
    interfaces->engine->getScreenSize(w, h);
    //unhook button
    ImGui::SetNextWindowPos(ImVec2(300, 40), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(100.f, 55.f), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.2f);
    ImGui::Begin("unhook button", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    if (ImGui::Button("unhook", ImVec2(90.f, 40.f))) hooks->uninstall();
    ImGui::End();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration;
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::SetNextWindowPos(ImVec2(w/2-440, h/2-270), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(880.f, 540.f), ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);


    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::BeginChild("Left Bar", ImVec2(52, 488), true);
    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::InvisibleButton("", ImVec2(52, 52))) window3.curWindow = 0;
    ImGui::SetCursorPos(ImVec2(0, 54));
    if (ImGui::Button("home\nicon", ImVec2(50, 86))) window3.curWindow = 0;
    ImGui::SetCursorPos(ImVec2(0, 140));
    if (ImGui::Button("aimbot\nicon", ImVec2(50, 86))) window3.curWindow = 1;
    ImGui::SetCursorPos(ImVec2(0, 226));
    if (ImGui::Button("visuals\nicon", ImVec2(50, 86))) window3.curWindow = 2;
    ImGui::SetCursorPos(ImVec2(0, 312));
    if (ImGui::Button("inventory\nchanger\nicon", ImVec2(50, 86))) window3.curWindow = 3;
    ImGui::SetCursorPos(ImVec2(0, 398));
    if (ImGui::Button("misc\nicon", ImVec2(50, 88))) window3.curWindow = 4;
    ImGui::EndChild();
    ImGui::SetCursorPos(ImVec2(52, 0));
    ImGui::BeginChild("TopBar", ImVec2(0, 52));
    switch (window3.curWindow) {
    case 0: ImGui::SetCursorPos(ImVec2(396, 20)); ImGui::TextColored(ImVec4(1, 1, 1, 1), "HOME"); break;
    case 1: ImGui::SetCursorPos(ImVec2(160, 20)); ImGui::TextColored(ImVec4(1, 1, 1, 1), "AIM ASSISTANCE"); draw_list->AddLine(ImVec2(ImGui::GetWindowPos().x+414, ImGui::GetWindowPos().y+3), ImVec2(ImGui::GetWindowPos().x+414, ImGui::GetWindowPos().y+48), ImColor(ImVec4(1, 1, 1, 1))); ImGui::SetCursorPos(ImVec2(575, 20)); ImGui::TextColored(ImVec4(1, 1, 1, 1), "TRIGGERBOT"); break;
    
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.214, 0.214, 0.214, 1));
    ImGui::SetCursorPos(ImVec2(0, 488));
    ImGui::BeginChild("User Info", ImVec2(160, 52), true);
    ImGui::EndChild();
    

    draw_list->AddLine(ImVec2(ImGui::GetWindowPos().x + 52, ImGui::GetWindowPos().y), ImVec2(ImGui::GetWindowPos().x + 52, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y-52), col32, 2.0f);    //vertical line
    draw_list->AddLine(ImVec2(ImGui::GetWindowPos().x + 52, ImGui::GetWindowPos().y+52), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + 52), col32, 2.0f);    //horizontal line
    draw_list->AddLine(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + 488), ImVec2(ImGui::GetWindowPos().x + 160, ImGui::GetWindowPos().y + 488), col32, 2.0f);                          //user info border
    draw_list->AddLine(ImVec2(ImGui::GetWindowPos().x + 160, ImGui::GetWindowPos().y + 488), ImVec2(ImGui::GetWindowPos().x +160, ImGui::GetWindowPos().y + 540), col32, 2.0f);                     //^
    draw_list->AddCircleFilled(ImVec2(ImGui::GetWindowPos().x+26, ImGui::GetWindowPos().y + 514), 22.f, col32);
    ImGui::SetCursorPos(ImVec2(52, 52));
    ImGui::SetNextWindowBgAlpha(0);
    ImGui::BeginChild("Main Child", ImVec2(862, 436), true);
    ImGui::PopStyleColor();
    switch (window3.curWindow) {
    case 0: renderHomeWindow(true); break;
    case 1: renderAimAssistance(true); break;
    case 2: renderVisualsWindow(true); break;
    case 3: InventoryChanger::drawGUI(true); break;
    case 4: Misc::drawGUI(true); break;
    }
    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor();

    //main window
    //ImGui::SetNextWindowSize(ImVec2(807, 500), ImGuiCond_Once);
    //ImGui::SetNextWindowBgAlpha(1.0);
    //ImGui::Begin("newslippery.gg", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    //ImGui::PushFont(fonts.backgroundCubes);
    //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    //ImGui::SetCursorPos(ImVec2(0, 0));
    //ImGui::Text(window3.backgroundString);
    //ImGui::SetCursorPos(ImVec2(8, 8));
    //ImGui::PopStyleColor();
    //ImGui::PopFont();
    //slipperyMenuPos = (ImGui::GetWindowPos());
    //if (window3.curWindow == 0)
    //    beginHighlight(ImVec4(0.2, 0.2, 0.2, 1));
    //if (ImGui::Button(("Home"), ImVec2(125, 60))) {
    //    window3.curWindow = 0;
    //};
    //endHighlight();
    //ImGui::SameLine();

    //if (window3.curWindow == 1)
    //    beginHighlight(ImVec4(0.2, 0.2, 0.2, 1));
    //if (ImGui::Button(("Aim Assistance"), ImVec2(125, 60))) {
    //    window3.curWindow = 1;
    //};
    //endHighlight();
    //ImGui::SameLine();
    //if (window3.curWindow == 2)
    //    beginHighlight(ImVec4(0.2, 0.2, 0.2, 1));
    //if (ImGui::Button(("Visuals"), ImVec2(125, 60))) {
    //    window3.curWindow = 2;
    //};
    //endHighlight();

    //ImGui::SameLine();
    //if (window3.curWindow == 3)
    //    beginHighlight(ImVec4(0.2, 0.2, 0.2, 1));
    //if (ImGui::Button(("Inventory Changer"), ImVec2(125, 60))) {
    //    window3.curWindow = 3;
    //};
    //endHighlight();

    //ImGui::SameLine();
    //if (window3.curWindow == 4)
    //    beginHighlight(ImVec4(0.2, 0.2, 0.2, 1));
    //if (ImGui::Button(("Sound"), ImVec2(125, 60))) {
    //    window3.curWindow = 4;
    //};
    //endHighlight();

    //ImGui::SameLine();
    //if (window3.curWindow == 5)
    //    beginHighlight(ImVec4(0.2, 0.2, 0.2, 1));
    //if (ImGui::Button(("Misc"), ImVec2(125, 60))) {
    //    window3.curWindow = 5;
    //};
    //endHighlight();
    //ImGui::SetCursorPos(ImVec2(0, 72));
    //ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1, 0.1, 0.1, 0.7));
    //ImGui::BeginChild("##contentMenu", ImVec2(807, 500), true);
    //ImGui::PopStyleColor();
    //ImGui::SetCursorPos(ImVec2(8, 5));
    //switch (window3.curWindow) {
    //case 0: renderHomeWindow(true); break;
    //case 1: renderAimAssistance(true); /*Misc::drawMiscAimbot()*/; break;
    //case 2: renderVisualsWindow(true); /*Misc::drawMiscVisuals();*/ break;
    //case 3: InventoryChanger::drawGUI(true); break;
    //case 4: Sound::drawGUI(true); /*Misc::drawMiscSound(); */ break;
    //case 5: Misc::drawGUI(true); break;
    //}
    //ImGui::EndChild();
    ////sidebar
    //float wi = w;           //without this compiler says that conversion from 'int' to 'float' requires a narrowing conversion!!!!!
    //float he = h;
    //ImGui::SetNextWindowSize(ImVec2(250, h), ImGuiCond_Once);
    //ImGui::SetNextWindowPos(ImVec2(w - 50, 0), ImGuiCond_Once);
    //ImGui::SetNextWindowBgAlpha(0.85f);
    //ImGui::Begin("Right Sidebar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    //ImVec2 curWindowPos{ ImGui::GetWindowPos() };
    //ImGui::Text(std::to_string(curWindowPos[0]).c_str());
    //ImGui::Text(std::to_string(wi - 250).c_str());
    //ImGui::Separator();

    //ImGui::Text("Config");  //config menu here
    //                        //not using BeginChild() because it makes the config menu way too wide for the sidebar
    //static bool incrementalLoad = false; //incremental load means that instead of replacing all data of already loaded config 
    //                                     //with data of config that's being loaded it will instead add data from config that's 
    //                                     //about to be loaded to the config that's already loaded. i.e.
    //                                     //01001011+11010110=11011111 - incremental load
    //                                     //01001011+11010110=11011110 - non-incremental load

    //auto& configItems = config->getConfigs();
    //static int currentConfig = -1;
    //static std::u8string buffer;

    //timeToNextConfigRefresh -= ImGui::GetIO().DeltaTime;
    //if (timeToNextConfigRefresh <= 0.0f) {
    //    config->listConfigs();
    //    if (const auto it = std::find(configItems.begin(), configItems.end(), buffer); it != configItems.end())
    //        currentConfig = std::distance(configItems.begin(), it);
    //    timeToNextConfigRefresh = 0.1f;
    //}

    //if (static_cast<std::size_t>(currentConfig) >= configItems.size())
    //    currentConfig = -1;
    //ImGui::PushItemWidth(246);
    //if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
    //    auto& vector = *static_cast<std::vector<std::u8string>*>(data);
    //    *out_text = (const char*)vector[idx].c_str();
    //    return true;
    //    }, &configItems, configItems.size(), 6) && currentConfig != -1)
    //    buffer = configItems[currentConfig];
    //    if (ImGui::InputTextWithHint("", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
    //        if (currentConfig != -1)
    //            config->rename(currentConfig, buffer);
    //    }

    //    if (ImGui::Button("Create Config", { 246.0f, 20.0f })) {
    //        config->add(buffer.c_str());
    //    }

    //    if (ImGui::Button("Load Config", { 246.0f, 20.0f })) {
    //        config->load(currentConfig, incrementalLoad);
    //        updateColors();
    //        InventoryChanger::scheduleHudUpdate();
    //        Misc::updateClanTag(true);
    //    }

    //    if (ImGui::Button("         Save Config", { 210.f, 20.f }))
    //        config->save(currentConfig);


    //    ImGui::SameLine();
    //    ImGui::PushFont(fonts.icons);
    //    if (ImGui::Button(("B"), { 20.f, 20.f }))             //there should be a folder icon in place of the "a"
    //        config->openConfigDir();;   // config menu over
    //    ImGui::PopFont();
    //    if (ImGui::Button("Delete Config", { 246.0f, 20.0f })) {
    //        window.deleteConfirmation = true;
    //    }
    //    if (window.deleteConfirmation) {
    //        ImGui::SetNextWindowSize(ImVec2(300, 128), ImGuiCond_Once);
    //        ImGui::SetNextWindowPos(ImVec2((w / 2 - 150), (h / 2 - 64)), ImGuiCond_Once);
    //        ImGui::Begin("Delete confirmation", &window.deleteConfirmation, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    //        ImGui::Text("Are you sure you want to delete config");
    //        ImGui::TextColored(ImVec4(255, 0, 0, 1), (char*)buffer.c_str());
    //        ImGui::SameLine();
    //        ImGui::Text("?");
    //        if (ImGui::Button("Yes", ImVec2(140, 40))) {
    //            config->remove(currentConfig);
    //            window.deleteConfirmation = false;
    //        };
    //        ImGui::SameLine();
    //        if (ImGui::Button("No", ImVec2(140, 40))) {
    //            window.deleteConfirmation = false;
    //        };
    //        ImGui::End();
    //    }
    //    ImGui::Separator();
    //    if (ImGui::IsWindowHovered() or ImGui::GetMousePos().x > (wi - 250)) {
    //        ImGui::SetWindowPos("Right Sidebar", ImVec2(curWindowPos[0] - (curWindowPos[0] - (wi - 250)) / sidebarSpeed[0], 0));
    //    }       //the way code ^ and v works is it makes the position of the sidebar 
    //            //less/more by ((distance to the desired position)/30) every frame
    //    if (!ImGui::IsWindowHovered() and !((GetKeyState(VK_LBUTTON) & 0x8000) != 0) and ImGui::GetMousePos().x < (wi - 250)) {
    //        ImGui::SetWindowPos("Right Sidebar", ImVec2(curWindowPos[0] + ((wi - 30) - curWindowPos[0] + sidebarSpeed[0] - 1) / sidebarSpeed[0], 0));
    //    }       //listHovered is required to not make sidebar go back to it's default position when you hover over the configs list
    //            //without the +29 the sidebar doesn't return to its original place, it stops 29 pixels before it should :(
    //    ImGui::End();



}