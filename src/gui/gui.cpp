#include <algorithm>
#include <array>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <ShlObj.h>
#endif

#include "imgui/imgui_stdlib.h"

#include "gui.h"
#include "custom.h"

#include "../features/config/config.h"
#include "../features/inventory/inventorychanger.h"
#include "../features/legitbot/backtrack.h"
#include "../features/other/misc.h"
#include "../features/visuals/visuals.h"

#include "../hooking/hooking.h"

#include "../utils/helpers.h"
#include "../utils/interfaces.h"
#include "../utils/memory.h"

#include "../../lib/sdk/InputSystem.h"

#define IMGUI_DEFINE_MATH_OPERATORS

constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept {
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

GUI::GUI() noexcept {
    ImGui::StyleColorsDark(); // I am going to change default colors in imgui_draw.cpp because fuck you imgui (no, copy the StyleColoursDark function,
    // paste it inside ImGuiCustom and modify it there, and then implement it here as ImGuiCustom::StyleColoursMain(); for example;
    ImGuiStyle& style = ImGui::GetStyle();

    style.TabBorderSize = 1.0f;
    style.TabRounding = 0.0f;
    style.ScrollbarSize = 9.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "slipperygg_imgui.ini";
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImFontConfig cfg;
    cfg.SizePixels = 15.0f;

#ifdef _WIN32
    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path { pathToFonts };
        CoTaskMemFree(pathToFonts);

        //fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "segoeui.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.normal15px)
            io.Fonts->AddFontDefault(&cfg);

        cfg.MergeMode = true;
        static constexpr ImWchar symbol[] {
            0x2605, 0x2605, // ★
            0
        };
        io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
        cfg.MergeMode = false;
    }
#else
    fonts.normal15px = addFontFromVFONT("csgo/panorama/fonts/notosans-regular.vfont", 15.0f, Helpers::getFontGlyphRanges(), false);
#endif
    if (!fonts.normal15px)
        io.Fonts->AddFontDefault(&cfg);
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 17.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
}

void GUI::updateColors() const noexcept {
    switch (config->other.style.menuColors) {
    case 0: ImGui::StyleColorsDark();
        break;
    case 1: ImGui::StyleColorsLight();
        break;
    case 2: ImGui::StyleColorsClassic();
        break;
    }
}

void GUI::handleToggle() noexcept {
    if (Misc::isMenuKeyPressed()) {
        open = !open;
        if (!open)
            interfaces->inputSystem->resetInputState();
#ifndef _WIN32
        ImGui::GetIO().MouseDrawCursor = gui->open;
#endif
    }
}

void GUI::renderAimbotWindow() noexcept {
    ImGui::Checkbox("On key", &config->legitbot.aimbotOnKey);
    ImGui::SameLine();
    ImGui::PushID("Aimbot Key");
    ImGui::hotkey("", config->legitbot.aimbotKey);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushItemWidth(70.0f);
    ImGui::Combo("", &config->legitbot.aimbotKeyMode, "Hold\0Toggle\0");
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::Separator();
    static int currentCategory { 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon { 0 };
    ImGui::PushID(1);

    switch (currentCategory) {
    case 0:currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1:
    {
        static int currentPistol { 0 };
        static constexpr const char
            * pistols[] { "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->legitbot.aimbot[idx ? idx : 35].enabled) {
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
    case 2:
    {
        static int currentHeavy { 0 };
        static constexpr const char* heavies[] { "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->legitbot.aimbot[idx ? idx + 10 : 36].enabled) {
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
    case 3:
    {
        static int currentSmg { 0 };
        static constexpr const char* smgs[] { "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->legitbot.aimbot[idx ? idx + 16 : 37].enabled) {
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
    case 4:
    {
        static int currentRifle { 0 };
        static constexpr const char* rifles[] { "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->legitbot.aimbot[idx ? idx + 23 : 38].enabled) {
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
    ImGui::Checkbox("Enabled", &config->legitbot.aimbot[currentWeapon].enabled);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 220.0f);
    ImGui::Checkbox("Aimlock", &config->legitbot.aimbot[currentWeapon].aimlock);
    ImGui::Checkbox("Silent", &config->legitbot.aimbot[currentWeapon].silent);
    ImGui::Checkbox("Friendly fire", &config->legitbot.aimbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Visible only", &config->legitbot.aimbot[currentWeapon].visibleOnly);
    ImGui::Checkbox("Scoped only", &config->legitbot.aimbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->legitbot.aimbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->legitbot.aimbot[currentWeapon].ignoreSmoke);
    ImGui::Checkbox("Auto shot", &config->legitbot.aimbot[currentWeapon].autoShot);
    ImGui::Checkbox("Auto scope", &config->legitbot.aimbot[currentWeapon].autoScope);
    ImGui::Combo("Bone", &config->legitbot.aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(240.0f);
    ImGui::SliderFloat("Fov", &config->legitbot.aimbot[currentWeapon].fov, 0.0f, 255.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Smooth", &config->legitbot.aimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Max aim inaccuracy", &config->legitbot.aimbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max shot inaccuracy", &config->legitbot.aimbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::InputInt("Min damage", &config->legitbot.aimbot[currentWeapon].minDamage);
    config->legitbot.aimbot[currentWeapon].minDamage = std::clamp(config->legitbot.aimbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->legitbot.aimbot[currentWeapon].killshot);
    ImGui::Checkbox("Between shots", &config->legitbot.aimbot[currentWeapon].betweenShots);
    ImGui::Columns(1);
}

void GUI::renderTriggerbotWindow() noexcept {
    static int currentCategory { 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0Zeus x27\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon { 0 };
    ImGui::PushID(1);
    switch (currentCategory) {
    case 0:currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 5:currentWeapon = 39;
        ImGui::NewLine();
        break;

    case 1:
    {
        static int currentPistol { 0 };
        static constexpr const char
            * pistols[] { "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->legitbot.triggerbot[idx ? idx : 35].enabled) {
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
    case 2:
    {
        static int currentHeavy { 0 };
        static constexpr const char* heavies[] { "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->legitbot.triggerbot[idx ? idx + 10 : 36].enabled) {
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
    case 3:
    {
        static int currentSmg { 0 };
        static constexpr const char* smgs[] { "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->legitbot.triggerbot[idx ? idx + 16 : 37].enabled) {
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
    case 4:
    {
        static int currentRifle { 0 };
        static constexpr const char* rifles[] { "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->legitbot.triggerbot[idx ? idx + 23 : 38].enabled) {
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
    ImGui::Checkbox("Enabled", &config->legitbot.triggerbot[currentWeapon].enabled);
    ImGui::Separator();
    ImGui::hotkey("Hold Key", config->legitbot.triggerbotKey);
    ImGui::Checkbox("Friendly fire", &config->legitbot.triggerbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Scoped only", &config->legitbot.triggerbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->legitbot.triggerbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->legitbot.triggerbot[currentWeapon].ignoreSmoke);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::Combo("Hitgroup", &config->legitbot.triggerbot[currentWeapon].hitgroup, "All\0Head\0Chest\0Stomach\0Left arm\0Right arm\0Left leg\0Right leg\0");
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Shot delay", &config->legitbot.triggerbot[currentWeapon].shotDelay, 0, 250, "%d ms");
    ImGui::InputInt("Min damage", &config->legitbot.triggerbot[currentWeapon].minDamage);
    config->legitbot.triggerbot[currentWeapon].minDamage = std::clamp(config->legitbot.triggerbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->legitbot.triggerbot[currentWeapon].killshot);
    ImGui::SliderFloat("Burst Time", &config->legitbot.triggerbot[currentWeapon].burstTime, 0.0f, 0.5f, "%.3f s");
}

void GUI::renderBacktrackWindow() noexcept {
    ImGui::Checkbox("Enabled", &config->legitbot.backtrack.enabled);
    ImGui::Checkbox("Ignore smoke", &config->legitbot.backtrack.ignoreSmoke);
    ImGui::Checkbox("Recoil based fov", &config->legitbot.backtrack.recoilBasedFov);
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Time limit", &config->legitbot.backtrack.timeLimit, 1, 200, "%d ms");
    ImGui::PopItemWidth();
}

void GUI::renderAntiAimWindow() noexcept {
    ImGui::Checkbox("test", &config->antiAim.test);
}

void GUI::renderGlowWindow() noexcept {
    static int currentCategory { 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    constexpr std::array categories
    { "Allies", "Enemies", "Planting", "Defusing", "Local Player", "Weapons", "C4", "Planted C4", "Chickens", "Defuse Kits", "Projectiles", "Hostages",
        "Ragdolls" };
    ImGui::Combo("", &currentCategory, categories.data(), categories.size());
    ImGui::PopID();
    Config::Visuals::GlowItem* currentItem;
    if (currentCategory <= 3) {
        ImGui::SameLine();
        static int currentType { 0 };
        ImGui::PushID(1);
        ImGui::Combo("", &currentType, "All\0Visible\0Occluded\0");
        ImGui::PopID();
        auto& cfg = config->visuals.playerGlow[categories[currentCategory]];
        switch (currentType) {
        case 0: currentItem = &cfg.all;
            break;
        case 1: currentItem = &cfg.visible;
            break;
        case 2: currentItem = &cfg.occluded;
            break;
        }
    }
    else {
        currentItem = &config->visuals.glow[categories[currentCategory]];
    }

    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &currentItem->enabled);
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 150.0f);
    ImGui::Checkbox("Health based", &currentItem->healthBased);

    ImGuiCustom::colorPicker("Color", *currentItem);

    ImGui::NextColumn();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::Combo("Style", &currentItem->style, "Default\0Rim3d\0Edge\0Edge Pulse\0");

    ImGui::Columns(1);
}

void GUI::renderChamsWindow() noexcept {
    static int currentCategory { 0 };
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

    constexpr std::array categories { "Allies", "Enemies", "Planting", "Defusing", "Local player", "Weapons", "Hands", "Backtrack", "Sleeves" };

    ImGui::SameLine();

    if (material >= int(config->visuals.chams[categories[currentCategory]].materials.size()))
        ImGuiCustom::arrowButtonDisabled("##right", ImGuiDir_Right);
    else if (ImGui::ArrowButton("##right", ImGuiDir_Right))
        ++material;

    ImGui::SameLine();

    auto& chams { config->visuals.chams[categories[currentCategory]].materials[material - 1] };

    ImGui::Checkbox("Enabled", &chams.enabled);
    ImGui::Separator();
    ImGui::Checkbox("Health based", &chams.healthBased);
    ImGui::Checkbox("Blinking", &chams.blinking);
    ImGui::Combo("Material", &chams.material, "Normal\0Flat\0Animated\0Platinum\0Glass\0Chrome\0Crystal\0Silver\0Gold\0Plastic\0Glow\0Pearlescent\0Metallic\0");
    ImGui::Checkbox("Wireframe", &chams.wireframe);
    ImGui::Checkbox("Cover", &chams.cover);
    ImGui::Checkbox("Ignore-Z", &chams.ignorez);
    ImGuiCustom::colorPicker("Color", chams);
}

void GUI::renderAntiOBSWindow() noexcept {
    static std::size_t currentCategory;
    static auto currentItem = "All";

    constexpr auto getConfigShared = [](std::size_t category, const char* item) noexcept -> Shared& {
        switch (category) {
        case 0:
        default: return config->visuals.antiOBS.enemies[item];
        case 1: return config->visuals.antiOBS.allies[item];
        case 2: return config->visuals.antiOBS.weapons[item];
        case 3: return config->visuals.antiOBS.projectiles[item];
        case 4: return config->visuals.antiOBS.lootCrates[item];
        case 5: return config->visuals.antiOBS.otherEntities[item];
        }
    };

    constexpr auto getConfigPlayer = [](std::size_t category, const char* item) noexcept -> Player& {
        switch (category) {
        case 0:
        default: return config->visuals.antiOBS.enemies[item];
        case 1: return config->visuals.antiOBS.allies[item];
        }
    };

    if (ImGui::BeginListBox("##list", { 170.0f, 300.0f })) {
        constexpr std::array categories { "Enemies", "Allies", "Weapons", "Projectiles", "Loot Crates", "Other Entities" };

        for (std::size_t i = 0; i < categories.size(); ++i) {
            if (ImGui::Selectable(categories[i], currentCategory == i && std::string_view { currentItem } == "All")) {
                currentCategory = i;
                currentItem = "All";
            }

            if (ImGui::BeginDragDropSource()) {
                switch (i) {
                case 0:
                case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, "All"), sizeof(Player), ImGuiCond_Once);
                    break;
                case 2: ImGui::SetDragDropPayload("Weapon", &config->visuals.antiOBS.weapons["All"], sizeof(Weapon), ImGuiCond_Once);
                    break;
                case 3: ImGui::SetDragDropPayload("Projectile", &config->visuals.antiOBS.projectiles["All"], sizeof(Projectile), ImGuiCond_Once);
                    break;
                default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, "All"), sizeof(Shared), ImGuiCond_Once);
                    break;
                }
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                    const auto& data = *(Player*)payload->Data;

                    switch (i) {
                    case 0:
                    case 1: getConfigPlayer(i, "All") = data;
                        break;
                    case 2: config->visuals.antiOBS.weapons["All"] = data;
                        break;
                    case 3: config->visuals.antiOBS.projectiles["All"] = data;
                        break;
                    default: getConfigShared(i, "All") = data;
                        break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                    const auto& data = *(Weapon*)payload->Data;

                    switch (i) {
                    case 0:
                    case 1: getConfigPlayer(i, "All") = data;
                        break;
                    case 2: config->visuals.antiOBS.weapons["All"] = data;
                        break;
                    case 3: config->visuals.antiOBS.projectiles["All"] = data;
                        break;
                    default: getConfigShared(i, "All") = data;
                        break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                    const auto& data = *(Projectile*)payload->Data;

                    switch (i) {
                    case 0:
                    case 1: getConfigPlayer(i, "All") = data;
                        break;
                    case 2: config->visuals.antiOBS.weapons["All"] = data;
                        break;
                    case 3: config->visuals.antiOBS.projectiles["All"] = data;
                        break;
                    default: getConfigShared(i, "All") = data;
                        break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                    const auto& data = *(Shared*)payload->Data;

                    switch (i) {
                    case 0:
                    case 1: getConfigPlayer(i, "All") = data;
                        break;
                    case 2: config->visuals.antiOBS.weapons["All"] = data;
                        break;
                    case 3: config->visuals.antiOBS.projectiles["All"] = data;
                        break;
                    default: getConfigShared(i, "All") = data;
                        break;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PushID(i);
            ImGui::Indent();

            const auto items = [](std::size_t category) noexcept -> std::vector<const char*> {
                switch (category) {
                case 0:
                case 1: return { "Visible", "Occluded" };
                case 2: return { "Pistols", "SMGs", "Rifles", "Sniper Rifles", "Shotguns", "Machineguns", "Grenades", "Melee", "Other" };
                case 3:
                    return { "Flashbang", "HE Grenade", "Breach Charge", "Bump Mine", "Decoy Grenade", "Molotov", "TA Grenade", "Smoke Grenade", "Snowball" };
                case 4: return { "Pistol Case", "Light Case", "Heavy Case", "Explosive Case", "Tools Case", "Cash Dufflebag" };
                case 5:
                    return { "Defuse Kit", "Chicken", "Planted C4", "Hostage", "Sentry", "Cash", "Ammo Box", "Radar Jammer", "Snowball Pile",
                        "Collectable Coin" };
                default: return {};
                }
            }(i);

            const auto categoryEnabled = getConfigShared(i, "All").enabled;

            for (std::size_t j = 0; j < items.size(); ++j) {
                static bool selectedSubItem;
                if (!categoryEnabled || getConfigShared(i, items[j]).enabled) {
                    if (ImGui::Selectable(items[j], currentCategory == i && !selectedSubItem && std::string_view { currentItem } == items[j])) {
                        currentCategory = i;
                        currentItem = items[j];
                        selectedSubItem = false;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        switch (i) {
                        case 0:
                        case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, items[j]), sizeof(Player), ImGuiCond_Once);
                            break;
                        case 2: ImGui::SetDragDropPayload("Weapon", &config->visuals.antiOBS.weapons[items[j]], sizeof(Weapon), ImGuiCond_Once);
                            break;
                        case 3: ImGui::SetDragDropPayload("Projectile", &config->visuals.antiOBS.projectiles[items[j]], sizeof(Projectile), ImGuiCond_Once);
                            break;
                        default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, items[j]), sizeof(Shared), ImGuiCond_Once);
                            break;
                        }
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;

                            switch (i) {
                            case 0:
                            case 1: getConfigPlayer(i, items[j]) = data;
                                break;
                            case 2: config->visuals.antiOBS.weapons[items[j]] = data;
                                break;
                            case 3: config->visuals.antiOBS.projectiles[items[j]] = data;
                                break;
                            default: getConfigShared(i, items[j]) = data;
                                break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;

                            switch (i) {
                            case 0:
                            case 1: getConfigPlayer(i, items[j]) = data;
                                break;
                            case 2: config->visuals.antiOBS.weapons[items[j]] = data;
                                break;
                            case 3: config->visuals.antiOBS.projectiles[items[j]] = data;
                                break;
                            default: getConfigShared(i, items[j]) = data;
                                break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;

                            switch (i) {
                            case 0:
                            case 1: getConfigPlayer(i, items[j]) = data;
                                break;
                            case 2: config->visuals.antiOBS.weapons[items[j]] = data;
                                break;
                            case 3: config->visuals.antiOBS.projectiles[items[j]] = data;
                                break;
                            default: getConfigShared(i, items[j]) = data;
                                break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;

                            switch (i) {
                            case 0:
                            case 1: getConfigPlayer(i, items[j]) = data;
                                break;
                            case 2: config->visuals.antiOBS.weapons[items[j]] = data;
                                break;
                            case 3: config->visuals.antiOBS.projectiles[items[j]] = data;
                                break;
                            default: getConfigShared(i, items[j]) = data;
                                break;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                if (i != 2)
                    continue;

                ImGui::Indent();

                const auto subItems = [](std::size_t item) noexcept -> std::vector<const char*> {
                    switch (item) {
                    case 0:
                        return { "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-SeveN", "CZ75-Auto", "Desert Eagle", "R8 Revolver" };
                    case 1: return { "MAC-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };
                    case 2: return { "Galil AR", "FAMAS", "AK-47", "M4A4", "M4A1-S", "SG 553", "AUG" };
                    case 3: return { "SSG 08", "AWP", "G3SG1", "SCAR-20" };
                    case 4: return { "Nova", "XM1014", "Sawed-Off", "MAG-7" };
                    case 5: return { "M249", "Negev" };
                    case 6:
                        return { "Flashbang", "HE Grenade", "Smoke Grenade", "Molotov", "Decoy Grenade", "Incendiary", "TA Grenade", "Fire Bomb",
                            "Diversion", "Frag Grenade", "Snowball" };
                    case 7: return { "Axe", "Hammer", "Wrench" };
                    case 8: return { "C4", "Healthshot", "Bump Mine", "Zone Repulsor", "Shield" };
                    default: return {};
                    }
                }(j);

                const auto itemEnabled = getConfigShared(i, items[j]).enabled;

                for (const auto subItem : subItems) {
                    auto& subItemConfig = config->visuals.antiOBS.weapons[subItem];
                    if ((categoryEnabled || itemEnabled) && !subItemConfig.enabled)
                        continue;

                    if (ImGui::Selectable(subItem, currentCategory == i && selectedSubItem && std::string_view { currentItem } == subItem)) {
                        currentCategory = i;
                        currentItem = subItem;
                        selectedSubItem = true;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload("Weapon", &subItemConfig, sizeof(Weapon), ImGuiCond_Once);
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;
                            subItemConfig = data;
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                ImGui::Unindent();
            }
            ImGui::Unindent();
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    ImGui::SameLine();

    if (ImGui::BeginChild("##child", { 400.0f, 0.0f })) {
        auto& sharedConfig = getConfigShared(currentCategory, currentItem);

        ImGui::Checkbox("Enabled", &sharedConfig.enabled);
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 260.0f);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::BeginCombo("Font", config->getSystemFonts()[sharedConfig.font.index].c_str())) {
            for (size_t i = 0; i < config->getSystemFonts().size(); i++) {
                bool isSelected = config->getSystemFonts()[i] == sharedConfig.font.name;
                if (ImGui::Selectable(config->getSystemFonts()[i].c_str(), isSelected, 0, { 250.0f, 0.0f })) {
                    sharedConfig.font.index = i;
                    sharedConfig.font.name = config->getSystemFonts()[i];
                    config->scheduleFontLoad(sharedConfig.font.name);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        constexpr auto spacing = 250.0f;
        ImGuiCustom::colorPicker("Snapline", sharedConfig.snapline);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::Combo("##1", &sharedConfig.snapline.type, "Bottom\0Top\0Crosshair\0");
        ImGui::SameLine(spacing);
        ImGuiCustom::colorPicker("Box", sharedConfig.box);
        ImGui::SameLine();

        ImGui::PushID("Box");

        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("Type", &sharedConfig.box.type, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
            ImGui::SetNextItemWidth(275.0f);
            ImGui::SliderFloat3("Scale", sharedConfig.box.scale.data(), 0.0f, 0.50f, "%.2f");
            ImGuiCustom::colorPicker("Fill", sharedConfig.box.fill);
            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGuiCustom::colorPicker("Name", sharedConfig.name);
        ImGui::SameLine(spacing);

        if (currentCategory < 2) {
            auto& playerConfig = getConfigPlayer(currentCategory, currentItem);

            ImGuiCustom::colorPicker("Weapon", playerConfig.weapon);
            ImGuiCustom::colorPicker("Flash Duration", playerConfig.flashDuration);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Skeleton", playerConfig.skeleton);
            ImGui::Checkbox("Audible Only", &playerConfig.audibleOnly);
            ImGui::SameLine(spacing);
            ImGui::Checkbox("Spotted Only", &playerConfig.spottedOnly);

            ImGuiCustom::colorPicker("Head Box", playerConfig.headBox);
            ImGui::SameLine();

            ImGui::PushID("Head Box");

            if (ImGui::Button("..."))
                ImGui::OpenPopup("");

            if (ImGui::BeginPopup("")) {
                ImGui::SetNextItemWidth(95.0f);
                ImGui::Combo("Type", &playerConfig.headBox.type, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
                ImGui::SetNextItemWidth(275.0f);
                ImGui::SliderFloat3("Scale", playerConfig.headBox.scale.data(), 0.0f, 0.50f, "%.2f");
                ImGuiCustom::colorPicker("Fill", playerConfig.headBox.fill);
                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::SameLine(spacing);
            ImGui::Checkbox("Health Bar", &playerConfig.healthBar.enabled);
            ImGui::SameLine();

            ImGui::PushID("Health Bar");

            if (ImGui::Button("..."))
                ImGui::OpenPopup("");

            if (ImGui::BeginPopup("")) {
                ImGui::SetNextItemWidth(95.0f);
                ImGui::Combo("Type", &playerConfig.healthBar.type, "Gradient\0Solid\0Health-based\0");
                if (playerConfig.healthBar.type == HealthBar::Solid) {
                    ImGui::SameLine();
                    ImGuiCustom::colorPicker("", playerConfig.healthBar.asColor4());
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
        else if (currentCategory == 2) {
            auto& weaponConfig = config->visuals.antiOBS.weapons[currentItem];
            ImGuiCustom::colorPicker("Ammo", weaponConfig.ammo);
        }
        else if (currentCategory == 3) {
            auto& trails = config->visuals.antiOBS.projectiles[currentItem].trails;

            ImGui::Checkbox("Trails", &trails.enabled);
            ImGui::SameLine(spacing + 77.0f);
            ImGui::PushID("Trails");

            if (ImGui::Button("..."))
                ImGui::OpenPopup("");

            if (ImGui::BeginPopup("")) {
                constexpr auto trailPicker = [](const char* name, Trail& trail) noexcept {
                    ImGui::PushID(name);
                    ImGuiCustom::colorPicker(name, trail);
                    ImGui::SameLine(150.0f);
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::Combo("", &trail.type, "Line\0Circles\0Filled Circles\0");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::InputFloat("Time", &trail.time, 0.1f, 0.5f, "%.1fs");
                    trail.time = std::clamp(trail.time, 1.0f, 60.0f);
                    ImGui::PopID();
                };

                trailPicker("Local Player", trails.localPlayer);
                trailPicker("Allies", trails.allies);
                trailPicker("Enemies", trails.enemies);
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::SetNextItemWidth(95.0f);
        ImGui::InputFloat("Text Cull Distance", &sharedConfig.textCullDistance, 0.4f, 0.8f, "%.1fm");
        sharedConfig.textCullDistance = std::clamp(sharedConfig.textCullDistance, 0.0f, 999.9f);
    }

    ImGui::EndChild();
}

void GUI::renderVisualsWindow() noexcept {
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 280.0f);
    ImGui::Checkbox("Disable Post Processing", &config->visuals.misc.disablePostProcessing);
    ImGui::Checkbox("Inverse Ragdoll Gravity", &config->visuals.misc.inverseRagdollGravity);
    ImGui::Checkbox("No Fog", &config->visuals.misc.noFog);
    ImGui::Checkbox("No 3D Sky", &config->visuals.misc.no3dSky);
    ImGui::Checkbox("No Aim Punch", &config->visuals.misc.noAimPunch);
    ImGui::Checkbox("No View Punch", &config->visuals.misc.noViewPunch);
    ImGui::Checkbox("No Hands", &config->visuals.misc.noHands);
    ImGui::Checkbox("No Sleeves", &config->visuals.misc.noSleeves);
    ImGui::Checkbox("No Weapons", &config->visuals.misc.noWeapons);
    ImGui::Checkbox("No Smoke", &config->visuals.misc.noSmoke);
    ImGui::Checkbox("No Blur", &config->visuals.misc.noBlur);
    ImGui::Checkbox("No Scope Overlay", &config->visuals.misc.noScopeOverlay);
    ImGui::Checkbox("No Grass", &config->visuals.misc.noGrass);
    ImGui::Checkbox("No Shadows", &config->visuals.misc.noShadows);
    ImGui::Checkbox("Wireframe Smoke", &config->visuals.misc.wireframeSmoke);
    ImGui::NextColumn();
    /*
    ImGui::Checkbox("Zoom", &config->visuals.zoom); // Who the fuck uses Zoom???
    ImGui::SameLine();
    ImGui::PushID("Zoom Key");
    ImGui::hotkey("", config->visuals.zoomKey);
    ImGui::PopID();
     */
    ImGui::Checkbox("Thirdperson", &config->visuals.misc.thirdperson);
    ImGui::SameLine();
    ImGui::PushID("Thirdperson Key");
    ImGui::hotkey("", config->visuals.misc.thirdpersonKey);
    ImGui::PopID();
    ImGui::PushItemWidth(290.0f);
    ImGui::PushID(0);
    ImGui::SliderInt("", &config->visuals.misc.thirdpersonDistance, 0, 1000, "Thirdperson distance: %d");
    ImGui::PopID();
    ImGui::PushID(1);
    ImGui::Checkbox("Viewmodel Modulation", &config->visuals.misc.viewmodelModulation);
    ImGui::SameLine();

    ImGui::PushID("Viewmodel Modulation");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SliderInt("Viewmodel FOV", &config->visuals.misc.viewmodelFov, 30, 140);
        ImGui::SliderFloat("Viewmodel offset X", &config->visuals.misc.viewmodelOffsetX, -10, 10);
        ImGui::SliderFloat("Viewmodel offset Y", &config->visuals.misc.viewmodelOffsetY, -10, 10);
        ImGui::SliderFloat("Viewmodel offset Z", &config->visuals.misc.viewmodelOffsetZ, -10, 10);
    }
    ImGui::PopID();
    ImGui::PushID(2);
    ImGui::SliderInt("", &config->visuals.misc.fov, -60, 60, "FOV: %d");
    ImGui::PopID();
    ImGui::PushID(3);
    ImGui::SliderInt("", &config->visuals.misc.farZ, 0, 2000, "Far Z: %d");
    ImGui::PopID();
    ImGui::PushID(4);
    ImGui::SliderInt("", &config->visuals.misc.flashReduction, 0, 100, "Flash reduction: %d%%");
    ImGui::PopID();
    ImGui::PushID(5);
    ImGui::SliderFloat("", &config->visuals.misc.brightness, 0.0f, 1.0f, "Brightness: %.2f");
    ImGui::PopID();
    ImGui::PopItemWidth();
    ImGui::Combo("Skybox", &config->visuals.misc.skybox, Visuals::skyboxList.data(), Visuals::skyboxList.size());
    ImGuiCustom::colorPicker("World color", config->visuals.misc.world);
    ImGuiCustom::colorPicker("Sky color", config->visuals.misc.sky);
    ImGui::Checkbox("Deagle spinner", &config->visuals.misc.deagleSpinner);
    ImGui::Combo("Screen effect", &config->visuals.misc.screenEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::Combo("Hit effect", &config->visuals.misc.hitEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::SliderFloat("Hit effect time", &config->visuals.misc.hitEffectTime, 0.1f, 1.5f, "%.2fs");
    ImGui::Combo("Hit marker", &config->visuals.misc.hitMarker, "None\0Default (Cross)\0");
    ImGui::SliderFloat("Hit marker time", &config->visuals.misc.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
    ImGuiCustom::colorPicker("Bullet Tracers",
                             config->visuals.misc.bulletTracers.asColor4().color.data(),
                             &config->visuals.misc.bulletTracers.asColor4().color[3],
                             nullptr,
                             nullptr,
                             &config->visuals.misc.bulletTracers.enabled);
    ImGuiCustom::colorPicker("Molotov Hull", config->visuals.misc.molotovHull);

    ImGui::Checkbox("Color correction", &config->visuals.misc.colorCorrection.enabled);
    ImGui::SameLine();

    if (bool ccPopup = ImGui::Button("Edit"))
        ImGui::OpenPopup("##popup");

    if (ImGui::BeginPopup("##popup")) {
        ImGui::VSliderFloat("##1", { 40.0f, 160.0f }, &config->visuals.misc.colorCorrection.blue, 0.0f, 1.0f, "Blue\n%.3f");
        ImGui::SameLine();
        ImGui::VSliderFloat("##2", { 40.0f, 160.0f }, &config->visuals.misc.colorCorrection.red, 0.0f, 1.0f, "Red\n%.3f");
        ImGui::SameLine();
        ImGui::VSliderFloat("##3", { 40.0f, 160.0f }, &config->visuals.misc.colorCorrection.mono, 0.0f, 1.0f, "Mono\n%.3f");
        ImGui::SameLine();
        ImGui::VSliderFloat("##4", { 40.0f, 160.0f }, &config->visuals.misc.colorCorrection.saturation, 0.0f, 1.0f, "Sat\n%.3f");
        ImGui::SameLine();
        ImGui::VSliderFloat("##5", { 40.0f, 160.0f }, &config->visuals.misc.colorCorrection.ghost, 0.0f, 1.0f, "Ghost\n%.3f");
        ImGui::SameLine();
        ImGui::VSliderFloat("##6", { 40.0f, 160.0f }, &config->visuals.misc.colorCorrection.green, 0.0f, 1.0f, "Green\n%.3f");
        ImGui::SameLine();
        ImGui::VSliderFloat("##7", { 40.0f, 160.0f }, &config->visuals.misc.colorCorrection.yellow, 0.0f, 1.0f, "Yellow\n%.3f");
        ImGui::SameLine();
        ImGui::EndPopup();
    }

    ImGuiCustom::colorPicker("Recoil Crosshair", config->other.misc.recoilCrosshair);
    ImGui::Columns(1);
}

void GUI::renderSoundWindow() noexcept {
    ImGui::PushID("Sound");
    ImGui::SliderInt("Chicken volume", &config->other.sound.chickenVolume, 0, 200, "%d%%");

    static int currentCategory { 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::Combo("", &currentCategory, "Local player\0Allies\0Enemies\0");
    ImGui::PopItemWidth();
    ImGui::SliderInt("Master volume", &config->other.sound.players[currentCategory].masterVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Headshot volume", &config->other.sound.players[currentCategory].headshotVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Weapon volume", &config->other.sound.players[currentCategory].weaponVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Footstep volume", &config->other.sound.players[currentCategory].footstepVolume, 0, 200, "%d%%");
    ImGui::PopID();
}

void GUI::renderMiscWindow() noexcept {
    ImGui::Columns(3, nullptr, false);
    ImGui::SetColumnOffset(1, 280.0f);
    ImGui::Checkbox("Anti AFK kick", &config->other.misc.antiAfkKick);
    ImGui::Checkbox("Auto strafe", &config->other.misc.autoStrafe);
    ImGui::Checkbox("Bunny hop", &config->other.misc.bunnyHop);
    ImGui::Checkbox("Fast duck", &config->other.misc.fastDuck);
    ImGui::Checkbox("Moonwalk", &config->other.misc.moonwalk);
    ImGui::Checkbox("Edge Jump", &config->other.misc.edgejump);
    ImGui::SameLine();
    ImGui::PushID("Edge Jump Key");
    ImGui::hotkey("", config->other.misc.edgejumpkey);
    ImGui::PopID();
    ImGui::Checkbox("Slow Walk", &config->other.misc.slowwalk); // Why the rename and unbind shift function?
    ImGui::SameLine();
    ImGui::PushID("Slowwalk Key");
    ImGui::hotkey("", config->other.misc.slowwalkKey);
    ImGui::PopID();
    ImGui::Checkbox("Auto pistol", &config->other.misc.autoPistol);
    ImGui::Checkbox("Auto reload", &config->other.misc.autoReload);
    ImGui::Checkbox("Auto accept", &config->other.misc.autoAccept);
    ImGui::Checkbox("Radar hack", &config->other.misc.radarHack);
    ImGui::Checkbox("Reveal ranks", &config->other.misc.revealRanks);
    ImGui::Checkbox("Reveal money", &config->other.misc.revealMoney);
    ImGui::Checkbox("Reveal suspect", &config->other.misc.revealSuspect);
    ImGui::Checkbox("Reveal votes", &config->other.misc.revealVotes);

    ImGui::Checkbox("Spectator list", &config->other.misc.spectatorList.enabled);
    ImGui::SameLine();

    ImGui::PushID("Spectator list");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("No Title Bar", &config->other.misc.spectatorList.noTitleBar);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Watermark", &config->other.misc.watermark.enabled);
    ImGuiCustom::colorPicker("Offscreen Enemies", config->other.misc.offscreenEnemies.asColor4(), &config->other.misc.offscreenEnemies.enabled);
    ImGui::SameLine();
    ImGui::PushID("Offscreen Enemies");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("Health Bar", &config->other.misc.offscreenEnemies.healthBar.enabled);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(95.0f);
        ImGui::Combo("Type", &config->other.misc.offscreenEnemies.healthBar.type, "Gradient\0Solid\0Health-based\0");
        if (config->other.misc.offscreenEnemies.healthBar.type == HealthBar::Solid) {
            ImGui::SameLine();
            ImGuiCustom::colorPicker("", config->other.misc.offscreenEnemies.healthBar.asColor4());
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::Checkbox("Fix animation LOD", &config->other.misc.fixAnimationLOD);
    ImGui::Checkbox("Fix bone matrix", &config->other.misc.fixBoneMatrix);
    ImGui::NextColumn();
    ImGui::Checkbox("Fix movement", &config->other.misc.fixMovement);
    ImGui::Checkbox("Disable model occlusion", &config->other.misc.disableModelOcclusion);
    ImGui::SliderFloat("Aspect Ratio", &config->other.misc.aspectratio, 0.0f, 5.0f, "%.2f");
    ImGui::Checkbox("Disable HUD blur", &config->other.misc.disablePanoramablur);
    ImGui::Checkbox("Animated clan tag", &config->other.misc.animatedClanTag);
    ImGui::Combo("Animation Type: ", &config->other.misc.tagAnimationType, "Rotate text\0Input from file\0");
    ImGui::Checkbox("Clock tag", &config->other.misc.clocktag);
    ImGui::Checkbox("Custom clantag", &config->other.misc.customClanTag);
    ImGui::SameLine();
    ImGui::PushItemWidth(120.0f);
    ImGui::PushID(0);

    if (ImGui::InputText("", config->other.misc.clanTag, sizeof(config->other.misc.clanTag)))
        Misc::updateClanTag(true);
    ImGui::PopID();
    ImGui::Checkbox("Kill message", &config->other.misc.killMessage);
    ImGui::SameLine();
    ImGui::PushItemWidth(120.0f);
    ImGui::PushID(1);
    ImGui::InputText("", &config->other.misc.killMessageString);
    ImGui::PopID();
    ImGui::Checkbox("Name stealer", &config->other.misc.nameStealer);
    ImGui::Checkbox("Fast Stop", &config->other.misc.fastStop);
    ImGuiCustom::colorPicker("Bomb timer", config->other.misc.bombTimer);
    ImGui::Checkbox("Quick reload", &config->other.misc.quickReload);
    ImGui::Checkbox("Prepare revolver", &config->other.misc.prepareRevolver);
    ImGui::SameLine();
    ImGui::PushID("Prepare revolver Key");
    ImGui::hotkey("", config->other.misc.prepareRevolverKey);
    ImGui::PopID();
    ImGui::Combo("Hit Sound", &config->other.misc.hitSound, "None\0Metal\0Gamesense\0Bell\0Glass\0Custom\0");
    if (config->other.misc.hitSound == 5) {
        ImGui::InputText("Hit Sound filename", &config->other.misc.customHitSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PushID(5);
    ImGui::Combo("Kill Sound", &config->other.misc.killSound, "None\0Metal\0Gamesense\0Bell\0Glass\0Custom\0");
    if (config->other.misc.killSound == 5) {
        ImGui::InputText("Kill Sound filename", &config->other.misc.customKillSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PopID();
    ImGui::SetNextItemWidth(90.0f);
    ImGui::InputInt("Choked packets", &config->other.misc.chokedPackets, 1, 5);
    config->other.misc.chokedPackets = std::clamp(config->other.misc.chokedPackets, 0, 64);
    ImGui::SameLine();
    ImGui::PushID("Choked packets Key");
    ImGui::hotkey("", config->other.misc.chokedPacketsKey);
    ImGui::PopID();
    ImGui::Checkbox("Grenade Prediction", &config->other.misc.nadePredict);
    ImGui::Checkbox("Fix tablet signal", &config->other.misc.fixTabletSignal);
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("Max angle delta", &config->other.misc.maxAngleDelta, 0.0f, 255.0f, "%.2f");
    ImGui::Checkbox("Opposite Hand Knife", &config->other.misc.oppositeHandKnife);
    ImGui::NextColumn();
    ImGui::Checkbox("Preserve Killfeed", &config->other.misc.preserveKillfeed.enabled);
    ImGui::SameLine();

    ImGui::PushID("Preserve Killfeed");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::Checkbox("Only Headshots", &config->other.misc.preserveKillfeed.onlyHeadshots);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Purchase List", &config->other.misc.purchaseList.enabled);
    ImGui::SameLine();

    ImGui::PushID("Purchase List");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SetNextItemWidth(75.0f);
        ImGui::Combo("Mode", &config->other.misc.purchaseList.mode, "Details\0Summary\0");
        ImGui::Checkbox("Only During Freeze Time", &config->other.misc.purchaseList.onlyDuringFreezeTime);
        ImGui::Checkbox("Show Prices", &config->other.misc.purchaseList.showPrices);
        ImGui::Checkbox("No Title Bar", &config->other.misc.purchaseList.noTitleBar);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Reportbot", &config->other.misc.reportbot.enabled);
    ImGui::SameLine();
    ImGui::PushID("Reportbot");

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::PushItemWidth(80.0f);
        ImGui::Combo("Target", &config->other.misc.reportbot.target, "Enemies\0Allies\0All\0");
        ImGui::InputInt("Delay (s)", &config->other.misc.reportbot.delay);
        config->other.misc.reportbot.delay = (std::max)(config->other.misc.reportbot.delay, 1);
        ImGui::InputInt("Rounds", &config->other.misc.reportbot.rounds);
        config->other.misc.reportbot.rounds = (std::max)(config->other.misc.reportbot.rounds, 1);
        ImGui::PopItemWidth();
        ImGui::Checkbox("Abusive Communications", &config->other.misc.reportbot.textAbuse);
        ImGui::Checkbox("Griefing", &config->other.misc.reportbot.griefing);
        ImGui::Checkbox("Wall Hacking", &config->other.misc.reportbot.wallhack);
        ImGui::Checkbox("Aim Hacking", &config->other.misc.reportbot.aimbot);
        ImGui::Checkbox("Other Hacking", &config->other.misc.reportbot.other);
        if (ImGui::Button("Reset"))
            Misc::resetReportbot();
        ImGui::EndPopup();
    }
    ImGui::PopID();

    if (ImGui::Button("Unhook"))
        hooks->uninstall();

    ImGui::Columns(1);
}

void GUI::renderStyleWindow() noexcept {
    ImGui::PushItemWidth(150.0f);
    if (ImGui::Combo("Menu Theme", &config->other.style.menuColors, "Dark\0Light\0Classic\0Custom\0"))
        updateColors();

#ifdef _WIN32 // FIXME: Temporary until Vulkan and OpenGL texture rendering for Linux
    ImGui::Combo("Menu Background", &config->other.style.menuBackground, "Cheat Background\0Solid Colour\0");
#else
    ImGui::Checkbox("Menu Background", &config->other.style.menuBackground);
#endif
    ImGui::SameLine();
    ImGuiCustom::colorPicker("Background Colour", config->other.style.menuBackgroundColor.asColor4());

    ImGui::PopItemWidth();

    if (config->other.style.menuColors == 3) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3)
                ImGui::SameLine(220.0f * (i & 3));

            ImGuiCustom::colorPicker(ImGui::GetStyleColorName(i), (float*)&style.Colors[i], &style.Colors[i].w);
        }
    }
}

void GUI::renderConfigWindow() noexcept {
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 170.0f);

    static bool incrementalLoad = false;
    ImGui::Checkbox("Incremental Load", &incrementalLoad);

    ImGui::PushItemWidth(160.0f);

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

    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::u8string>*>(data);
        *out_text = (const char*)vector[idx].c_str();
        return true;
        }, &configItems, configItems.size(), 5) && currentConfig != -1)
        buffer = configItems[currentConfig];

        ImGui::PushID(0);
        if (ImGui::InputTextWithHint("", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (currentConfig != -1)
                config->rename(currentConfig, buffer);
        }
        ImGui::PopID();
        ImGui::NextColumn();

        ImGui::PushItemWidth(100.0f);

        if (ImGui::Button("Open config directory"))
            config->openConfigDir();

        if (ImGui::Button("Create config", { 100.0f, 25.0f }))
            config->add(buffer.c_str());

        if (ImGui::Button("Reset config", { 100.0f, 25.0f }))
            ImGui::OpenPopup("Config to reset");

        if (ImGui::BeginPopup("Config to reset")) {
            static constexpr const char* names[]
            { "Whole", "Aimbot", "Triggerbot", "Backtrack", "Anti aim", "Glow", "Chams", "ESP", "Visuals", "Inventory Changer", "Sound", "Style", "Misc" };
            for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
                if (i == 1)
                    ImGui::Separator();

                if (ImGui::Selectable(names[i])) {
                    switch (i) {
                    case 0: config->reset();
                        updateColors();
                        Misc::updateClanTag(true);
                        InventoryChanger::scheduleHudUpdate();
                        break;
                    case 1: config->legitbot.aimbot = {};
                          break;
                    case 2: config->legitbot.triggerbot = {};
                          break;
                    case 3: config->legitbot.backtrack = {};
                          break;
                    case 4: config->antiAim = {};
                          break;
                    case 5: config->visuals.glow = {};
                          break;
                    case 6: config->visuals.chams = {};
                          break;
                    case 7: config->visuals.antiOBS = {};
                          break;
                    case 8: config->visuals.misc = {};
                          break;
                    case 9: InventoryChanger::resetConfig();
                        InventoryChanger::scheduleHudUpdate();
                        break;
                    case 10: config->other.sound = {};
                           break;
                    case 11: config->other.misc = {};
                           Misc::updateClanTag(true);
                           break;
                    case 12: config->other.style = {};
                           updateColors();
                           break;
                    }
                }
            }
            ImGui::EndPopup();
        }
        if (currentConfig != -1) {
            if (ImGui::Button("Load selected", { 100.0f, 25.0f })) {
                config->load(currentConfig, incrementalLoad);
                updateColors();
                InventoryChanger::scheduleHudUpdate();
                Misc::updateClanTag(true);
            }
            if (ImGui::Button("Save selected", { 100.0f, 25.0f }))
                config->save(currentConfig);
            if (ImGui::Button("Delete selected", { 100.0f, 25.0f })) {
                config->remove(currentConfig);

                if (static_cast<std::size_t>(currentConfig) < configItems.size())
                    buffer = configItems[currentConfig];
                else
                    buffer.clear();
            }
        }
        ImGui::Columns(1);
}

void GUI::render() noexcept {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    auto displaySize = ImGui::GetIO().DisplaySize;

    if (config->other.style.menuBackground == 0) {
        //dl->AddImage((void*) hooks->texture, ImVec2(0, 0), displaySize);
    }
    else {
        //dl->AddRectFilled(ImVec2(0, 0), displaySize, ImGui::ColorConvertFloat4ToU32(config->other.style.menuBackgroundColor.asColor4().color));
    }

    ImGui::SetNextWindowSize((ImGui::GetIO().DisplaySize / 2), ImGuiCond_Once);
    ImGui::Begin("slippery.gg", nullptr, windowFlags);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
        // Legitbot

        if (ImGui::BeginTabItem("Aimbot")) {
            // TODO: renderLegitbotWindow();
            renderAimbotWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Triggerbot")) {
            renderTriggerbotWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Backtrack")) {
            renderBacktrackWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Anti Aim")) {
            renderAntiAimWindow();
            ImGui::EndTabItem();
        }

        // Visuals

        if (ImGui::BeginTabItem("Glow")) {
            renderGlowWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Chams")) {
            renderChamsWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Anti OBS")) {
            renderAntiOBSWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visuals")) {
            renderVisualsWindow();
            ImGui::EndTabItem();
        }

        // IC

        if (ImGui::BeginTabItem("Inventory Changer")) {
            InventoryChanger::renderInventoryChangerWindow();
            ImGui::EndTabItem();
        }

        // Other

        if (ImGui::BeginTabItem("Sound")) {
            renderSoundWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Misc")) {
            renderMiscWindow();
            ImGui::EndTabItem();
        }

        // Cheat Related

        if (ImGui::BeginTabItem("Style")) {
            renderStyleWindow();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Config")) {
            renderConfigWindow();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}
