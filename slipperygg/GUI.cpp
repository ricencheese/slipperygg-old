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
#include "Hacks/Visuals.h"
#include "Hacks/Glow.h"
#include "Hacks/AntiAim.h"
#include "Hacks/Backtrack.h"
#include "Hacks/Sound.h"
#include "Hacks/StreamProofESP.h"

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
    cfgbackground.SizePixels = 80.0f;
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
        fonts.backgroundCubes = io.Fonts->AddFontFromFileTTF((path / "slippery/dependencies/slippery.ttf").string().c_str(), 80.0f, &cfgbackground, Helpers::getFontGlyphRanges());
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
    renderGuiStyle2();
}
/*
void GUI::styleColorsDark() noexcept
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] =                 ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] =         ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] =             ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg] =              ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] =              ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] =               ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] =         ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] =              ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] =       ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive] =        ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg] =              ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] =        ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] =     ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] =            ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] =          ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] =        ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] =  ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] =            ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_SliderGrab] =           ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] =     ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] =               ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] =        ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] =         ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] =               ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] =        ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] =         ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] =            colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] =     ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] =      ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] =           ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] =    ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] =     ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] =                  ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] =           colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] =            ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused] =         ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] =   ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines] =            ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] =     ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] =        ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] =        ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] =    ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight] =     ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg] =           ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] =        ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] =       ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] =       ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] =         ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] =    ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] =     ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}
*/
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
        if (!&window.home)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Home", &window.home, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("sidebar speed", sidebarSpeed, 1, 200);
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Dark theme\0Light theme\0"))
        updateColors();
    ImGui::PopItemWidth();
    /*switch (idx)
    {
    ImGuiCol_Text: return "Text";
    ImGuiCol_TextDisabled: return "TextDisabled";
    ImGuiCol_WindowBg: return "WindowBg";
    ImGuiCol_ChildBg: return "ChildBg";
    ImGuiCol_PopupBg: return "PopupBg";
    ImGuiCol_Border: return "Border";
    ImGuiCol_BorderShadow: return "BorderShadow";
    ImGuiCol_FrameBg: return "FrameBg";
    ImGuiCol_FrameBgHovered: return "FrameBgHovered";
    ImGuiCol_FrameBgActive: return "FrameBgActive";
    ImGuiCol_TitleBg: return "TitleBg";
    ImGuiCol_TitleBgActive: return "TitleBgActive";
    ImGuiCol_TitleBgCollapsed: return "TitleBgCollapsed";
    ImGuiCol_MenuBarBg: return "MenuBarBg";
    ImGuiCol_ScrollbarBg: return "ScrollbarBg";
    ImGuiCol_ScrollbarGrab: return "ScrollbarGrab";
    ImGuiCol_ScrollbarGrabHovered: return "ScrollbarGrabHovered";
    ImGuiCol_ScrollbarGrabActive: return "ScrollbarGrabActive";
    ImGuiCol_CheckMark: return "CheckMark";
    ImGuiCol_SliderGrab: return "SliderGrab";
    ImGuiCol_SliderGrabActive: return "SliderGrabActive";
    ImGuiCol_Button: return "Button";
    ImGuiCol_ButtonHovered: return "ButtonHovered";
    ImGuiCol_ButtonActive: return "ButtonActive";
    ImGuiCol_Header: return "Header";
    ImGuiCol_HeaderHovered: return "HeaderHovered";
    ImGuiCol_HeaderActive: return "HeaderActive";
    ImGuiCol_Separator: return "Separator";
    ImGuiCol_SeparatorHovered: return "SeparatorHovered";
    ImGuiCol_SeparatorActive: return "SeparatorActive";
    ImGuiCol_ResizeGrip: return "ResizeGrip";
    ImGuiCol_ResizeGripHovered: return "ResizeGripHovered";
    ImGuiCol_ResizeGripActive: return "ResizeGripActive";
    ImGuiCol_Tab: return "Tab";
    ImGuiCol_TabHovered: return "TabHovered";
    ImGuiCol_TabActive: return "TabActive";
    ImGuiCol_TabUnfocused: return "TabUnfocused";
    ImGuiCol_TabUnfocusedActive: return "TabUnfocusedActive";
    ImGuiCol_PlotLines: return "PlotLines";
    ImGuiCol_PlotLinesHovered: return "PlotLinesHovered";
    ImGuiCol_PlotHistogram: return "PlotHistogram";
    ImGuiCol_PlotHistogramHovered: return "PlotHistogramHovered";
    ImGuiCol_TableHeaderBg: return "TableHeaderBg";
    ImGuiCol_TableBorderStrong: return "TableBorderStrong";
    ImGuiCol_TableBorderLight: return "TableBorderLight";
    ImGuiCol_TableRowBg: return "TableRowBg";
    ImGuiCol_TableRowBgAlt: return "TableRowBgAlt";
    ImGuiCol_TextSelectedBg: return "TextSelectedBg";
    ImGuiCol_DragDropTarget: return "DragDropTarget";
    ImGuiCol_NavHighlight: return "NavHighlight";
    ImGuiCol_NavWindowingHighlight: return "NavWindowingHighlight";
    ImGuiCol_NavWindowingDimBg: return "NavWindowingDimBg";
    ImGuiCol_ModalWindowDimBg: return "ModalWindowDimBg";*/
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderAimbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.aimbot)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("Aimbot", &window.aimbot, windowFlags);
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
            if (config->aimbot[idx ? idx + 10 : 36].enabled) {
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
            if (config->aimbot[idx ? idx + 16 : 37].enabled) {
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
            if (config->aimbot[idx ? idx + 23 : 38].enabled) {
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
    ImGui::Combo("Bone", &config->aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(240.0f);
    ImGui::SliderFloat("Fov", &config->aimbot[currentWeapon].fov, 0.0f, 255.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Smooth", &config->aimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Max aim inaccuracy", &config->aimbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max shot inaccuracy", &config->aimbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::InputInt("Min damage", &config->aimbot[currentWeapon].minDamage);
    config->aimbot[currentWeapon].minDamage = std::clamp(config->aimbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->aimbot[currentWeapon].killshot);
    ImGui::Checkbox("Between shots", &config->aimbot[currentWeapon].betweenShots);
    ImGui::Columns(1);
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
//next code renders the single window menu, we're going to be using this instead of the classic top of the screen menu, right?
void GUI::renderGuiStyle2() noexcept
{   

    //honestly this is retarded but it's 1:30am and I can't think of any other way to draw background cubes without
    //every option in the menu going off-screen. If you have a better solution, please fix it :^)
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
    ImGui::Begin("background", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar);
    ImGui::PushFont(fonts.backgroundCubes);
    ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.f), "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    ImGui::PopFont();
    ImGui::SetWindowPos(slipperyMenuPos);
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("slippery.gg", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar);
    slipperyMenuPos = (ImGui::GetWindowPos());
    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) { 
        if (ImGui::BeginTabItem("Home")) {
            renderHomeWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Aimbot")) {
            renderAimbotWindow(true);
            ImGui::EndTabItem();
        }
        //AntiAim::tabItem(); //antiaim in a legit cheat?? it's useless and dangerous (animfix doesn't show you aa'ing)
        if (ImGui::BeginTabItem("Triggerbot")) {
            renderTriggerbotWindow(true);
            ImGui::EndTabItem();
        }
        Backtrack::tabItem();
        Glow::tabItem();
        if (ImGui::BeginTabItem("Chams")) {
            renderChamsWindow(true);
            ImGui::EndTabItem();
        }
        StreamProofESP::tabItem();
        Visuals::tabItem();
        InventoryChanger::tabItem();
        Sound::tabItem();
        if (ImGui::BeginTabItem("Style")) {
            renderStyleWindow(true);
            ImGui::EndTabItem();
        }
        Misc::tabItem();//WHY THE FUCK DOES IT NOT ALLOW ME TO SWITCH FROM MISC MENU TO CONFIG MENU <- fixed
        /*if (ImGui::BeginTabItem("Config")) {
            renderConfigWindow(true);
            ImGui::EndTabItem();
        }*/
        ImGui::EndTabBar();
    }

    ImGui::End();

    int w, h;           //SLIDY SIDEBAR ON THE RIGHT
    interfaces->engine->getScreenSize(w, h);
    float wi = w;           //without this compiler says that conversion from 'int' to 'float' requires a narrowing conversion!!!!!
    float he = h;
    ImGui::SetNextWindowSize(ImVec2(250, h), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(w - 50, 0), ImGuiCond_Once);
    ImGui::Begin("Right Sidebar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImVec2 curWindowPos{ ImGui::GetWindowPos() };
    ImGui::Text(std::to_string(curWindowPos[0]).c_str());
    ImGui::Text(std::to_string(wi - 250).c_str());
    ImGui::Separator();

    ImGui::Text("Config");  //config menu here
                            //not using BeginChild() because it makes the config menu way too wide for the sidebar
    static bool incrementalLoad = false; //incremental load means that instead of replacing all data of already loaded config 
                                         //with data of config that's being loaded it will instead add data from config that's 
                                         //about to be loaded to the config that's already loaded. i.e.
                                         //01001011+11010110=11011111 - incremental load
                                         //01001011+11010110=11011110 - non-incremental load

    auto& configItems = config->getConfigs();
    static int currentConfig = -1;
    static std::u8string buffer;

    timeToNextConfigRefresh -= ImGui::GetIO().DeltaTime;
    if (timeToNextConfigRefresh <= 0.0f) {
        config->listConfigs();
        if (const auto it = std::find(configItems.begin(), configItems.end(), buffer); it != configItems.end())
            currentConfig = std::distance(configItems.begin(), it);
        timeToNextConfigRefresh = 0.1f;
    }

    if (static_cast<std::size_t>(currentConfig) >= configItems.size())
        currentConfig = -1;
    ImGui::PushItemWidth(246);
    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::u8string>*>(data);
        *out_text = (const char*)vector[idx].c_str();
        return true;
        }, &configItems, configItems.size(), 6) && currentConfig != -1)
        buffer = configItems[currentConfig];
        bool listHovered{ false };
        if (ImGui::IsItemHovered()) listHovered = true;
        else listHovered = false;
        if (ImGui::InputTextWithHint("", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (currentConfig != -1)
                config->rename(currentConfig, buffer);
        }

        if (ImGui::Button("Save Config", { 246.0f, 20.0f }))
            config->save(currentConfig);


        if (ImGui::Button("Load Config", { 210.f, 20.f })) {
            config->load(currentConfig, incrementalLoad);
            updateColors();
            InventoryChanger::scheduleHudUpdate();
            Misc::updateClanTag(true);
        }
        ImGui::SameLine();
        if (ImGui::Button(("a"), { 20.f, 20.f }))             //there should be a folder icon in place of the "a"
            config->openConfigDir();;   // config menu over
        ImGui::Separator();

        //TODO: Add a prompt when saving/loading a config (i.e. like legendware does it)
    if (ImGui::IsWindowHovered() or listHovered) {
        ImGui::SetWindowPos("Right Sidebar", ImVec2(curWindowPos[0] - (curWindowPos[0] - (wi - 250))/sidebarSpeed[0], 0));
    }       //the way code ^ and v works is it makes the position of the sidebar 
            //less/more by ((distance to the desired position)/30) every frame
    if (!ImGui::IsWindowHovered() and !listHovered) {
        ImGui::SetWindowPos("Right Sidebar", ImVec2(curWindowPos[0] + ((wi - 30) - curWindowPos[0]+sidebarSpeed[0] - 1) / sidebarSpeed[0], 0));
    }       //listHovered is required to not make sidebar go back to it's default position when you hover over the configs list
            //without the +29 the sidebar doesn't return to its original place, it stops 29 pixels before it should :(
    ImGui::End();
}

//TODO LIST:
//scrolling down the menu hides the top menu bar, maybe make menu taller/wider+move the options a bit?
//^ignore that, I'm going to make a button-based menu bar

//tabs with not enough options look retarded, maybe move tabs with a few options to bigger tabs?
//i.e. move backtracking to aimbot tab https://thats-crazy-but-i-dont.remember911.bar/b479a901be9743d77d.png

//https://thats-crazy-but-i-dont.remember911.bar/bf0df5f0c78f95ec4c.png sliders are too ugly :(

//ESP on key? why? it just takes up space https://thats-crazy-but-i-dont.remember911.bar/c493984b1f68179321.png


//options in the wrong place todo list:
//choked packets is in misc + choked packets is limited to 64 :face_with_raised_eyebrow: 
//putting antiaim, triggerbot and backtrack in aimbot tab would free up a lot of space, perhaps divide the aimbot tab into 4 tabs with diff features?
//^same for glow, chams, esp, inventory changer, sound and style
//https://thats-crazy-but-i-dont.remember911.bar/2c4fef225901e35c0b.png something like that 

//^my retarded ass forgot that I autodelete screenshots after 48 hours