#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#include <shellapi.h>
#include <ShlObj.h>
#endif

#include "config.h"

#include "../inventory/inventorychanger.h"
#include "../legitbot/backtrack.h"
#include "../other/sound.h"

#include "../../../lib/json/json.hpp"

#include "../../../lib/imgui/imgui.h"

#ifdef _WIN32
int CALLBACK fontCallback(const LOGFONTW* lpelfe, const TEXTMETRICW*, DWORD, LPARAM lParam) {
    const wchar_t* const fontName = reinterpret_cast<const ENUMLOGFONTEXW*>(lpelfe)->elfFullName;

    if (fontName[0] == L'@')
        return TRUE;

    if (HFONT font = CreateFontW(0, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, fontName)) {

        DWORD fontData = GDI_ERROR;

        if (HDC hdc = CreateCompatibleDC(nullptr)) {
            SelectObject(hdc, font);
            // Do not use TTC fonts as we only support TTF fonts
            fontData = GetFontData(hdc, 'fctt', 0, NULL, 0);
            DeleteDC(hdc);
        }
        DeleteObject(font);

        if (fontData == GDI_ERROR) {
            if (char buff[1024]; WideCharToMultiByte(CP_UTF8, 0, fontName, -1, buff, sizeof(buff), nullptr, nullptr))
                reinterpret_cast<std::vector<std::string>*>(lParam)->emplace_back(buff);
        }
    }
    return TRUE;
}
#endif

[[nodiscard]] static std::filesystem::path buildConfigsFolderPath() noexcept {
    std::filesystem::path path;
#ifdef _WIN32
    if (PWSTR pathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathToDocuments))) {
        path = pathToDocuments;
        CoTaskMemFree(pathToDocuments);
    }
#else
    if (const char* homeDir = getenv("HOME"))
        path = homeDir;
#endif

    path /= "slipperygg";
    return path;
}

Config::Config() noexcept : path { buildConfigsFolderPath() } {
    listConfigs();

    load(u8"default.json", false);

#ifdef _WIN32
    LOGFONTW logfont;
    logfont.lfCharSet = ANSI_CHARSET;
    logfont.lfPitchAndFamily = DEFAULT_PITCH;
    logfont.lfFaceName[0] = L'\0';

    EnumFontFamiliesExW(GetDC(nullptr), &logfont, fontCallback, (LPARAM)&systemFonts, 0);
#endif

    std::sort(std::next(systemFonts.begin()), systemFonts.end());
}

static void from_json(const json& j, ColorToggleRounding& ctr) {
    from_json(j, static_cast<ColorToggle&>(ctr));

    read(j, "Rounding", ctr.rounding);
}

static void from_json(const json& j, Font& f) {
    read<value_t::string>(j, "Name", f.name);

    if (!f.name.empty())
        config->scheduleFontLoad(f.name);

    if (const auto it = std::ranges::find(config->getSystemFonts(), f.name); it != config->getSystemFonts().end())
        f.index = std::distance(config->getSystemFonts().begin(), it);
    else
        f.index = 0;
}

static void from_json(const json& j, Snapline& s) {
    from_json(j, static_cast<ColorToggleThickness&>(s));

    read(j, "Type", s.type);
}

static void from_json(const json& j, Box& b) {
    from_json(j, static_cast<ColorToggleRounding&>(b));

    read(j, "Type", b.type);
    read(j, "Scale", b.scale);
    read<value_t::object>(j, "Fill", b.fill);
}

static void from_json(const json& j, Shared& s) {
    read(j, "Enabled", s.enabled);
    read<value_t::object>(j, "Font", s.font);
    read<value_t::object>(j, "Snapline", s.snapline);
    read<value_t::object>(j, "Box", s.box);
    read<value_t::object>(j, "Name", s.name);
    read(j, "Text Cull Distance", s.textCullDistance);
}

static void from_json(const json& j, Weapon& w) {
    from_json(j, static_cast<Shared&>(w));

    read<value_t::object>(j, "Ammo", w.ammo);
}

static void from_json(const json& j, Trail& t) {
    from_json(j, static_cast<ColorToggleThickness&>(t));

    read(j, "Type", t.type);
    read(j, "Time", t.time);
}

static void from_json(const json& j, Trails& t) {
    read(j, "Enabled", t.enabled);
    read<value_t::object>(j, "Local Player", t.localPlayer);
    read<value_t::object>(j, "Allies", t.allies);
    read<value_t::object>(j, "Enemies", t.enemies);
}

static void from_json(const json& j, Projectile& p) {
    from_json(j, static_cast<Shared&>(p));

    read<value_t::object>(j, "Trails", p.trails);
}

static void from_json(const json& j, Player& p) {
    from_json(j, static_cast<Shared&>(p));

    read<value_t::object>(j, "Weapon", p.weapon);
    read<value_t::object>(j, "Flash Duration", p.flashDuration);
    read(j, "Audible Only", p.audibleOnly);
    read(j, "Spotted Only", p.spottedOnly);
    read<value_t::object>(j, "Health Bar", p.healthBar);
    read<value_t::object>(j, "Skeleton", p.skeleton);
    read<value_t::object>(j, "Head Box", p.headBox);
}

// Legitbot

static void from_json(const json& j, Config::Legitbot::Aimbot& a) {
    read(j, "Enabled", a.enabled);
    read(j, "Aimlock", a.aimlock);
    read(j, "Silent", a.silent);
    read(j, "Friendly fire", a.friendlyFire);
    read(j, "Visible only", a.visibleOnly);
    read(j, "Scoped only", a.scopedOnly);
    read(j, "Ignore flash", a.ignoreFlash);
    read(j, "Ignore smoke", a.ignoreSmoke);
    read(j, "Auto shot", a.autoShot);
    read(j, "Auto scope", a.autoScope);
    read(j, "Fov", a.fov);
    read(j, "Smooth", a.smooth);
    read(j, "Bone", a.bone);
    read(j, "Max aim inaccuracy", a.maxAimInaccuracy);
    read(j, "Max shot inaccuracy", a.maxShotInaccuracy);
    read(j, "Min damage", a.minDamage);
    read(j, "Killshot", a.killshot);
    read(j, "Between shots", a.betweenShots);
}

static void to_json(json& j, const Config::Legitbot::Aimbot& o, const Config::Legitbot::Aimbot& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Aimlock", aimlock);
    WRITE("Silent", silent);
    WRITE("Friendly fire", friendlyFire);
    WRITE("Visible only", visibleOnly);
    WRITE("Scoped only", scopedOnly);
    WRITE("Ignore flash", ignoreFlash);
    WRITE("Ignore smoke", ignoreSmoke);
    WRITE("Auto shot", autoShot);
    WRITE("Auto scope", autoScope);
    WRITE("Fov", fov);
    WRITE("Smooth", smooth);
    WRITE("Bone", bone);
    WRITE("Max aim inaccuracy", maxAimInaccuracy);
    WRITE("Max shot inaccuracy", maxShotInaccuracy);
    WRITE("Min damage", minDamage);
    WRITE("Killshot", killshot);
    WRITE("Between shots", betweenShots);
}

static void from_json(const json& j, Config::Legitbot::Triggerbot& t) {
    read(j, "Enabled", t.enabled);
    read(j, "Friendly fire", t.friendlyFire);
    read(j, "Scoped only", t.scopedOnly);
    read(j, "Ignore flash", t.ignoreFlash);
    read(j, "Ignore smoke", t.ignoreSmoke);
    read(j, "Hitgroup", t.hitgroup);
    read(j, "Shot delay", t.shotDelay);
    read(j, "Min damage", t.minDamage);
    read(j, "Killshot", t.killshot);
    read(j, "Burst Time", t.burstTime);
}

static void to_json(json& j, const Config::Legitbot::Triggerbot& o, const Config::Legitbot::Triggerbot& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Friendly fire", friendlyFire);
    WRITE("Scoped only", scopedOnly);
    WRITE("Ignore flash", ignoreFlash);
    WRITE("Ignore smoke", ignoreSmoke);
    WRITE("Hitgroup", hitgroup);
    WRITE("Shot delay", shotDelay);
    WRITE("Min damage", minDamage);
    WRITE("Killshot", killshot);
    WRITE("Burst Time", burstTime);
}

static void from_json(const json& j, Config::Legitbot::Backtrack& backtrack) {
    read(j, "Enabled", backtrack.enabled);
    read(j, "Ignore Smoke", backtrack.ignoreSmoke);
    read(j, "Recoil Based FOV", backtrack.recoilBasedFov);
    read(j, "Time Limit", backtrack.timeLimit);
}

static void to_json(json& j, const Config::Legitbot::Backtrack& o, const Config::Legitbot::Backtrack& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Ignore Smoke", ignoreSmoke);
    WRITE("Recoil Based FOV", recoilBasedFov);
    WRITE("Time Limit", timeLimit);
}

static void from_json(const json& j, Config::Legitbot& cfg) {
    read(j, "Aimbot", cfg.aimbot);
    read(j, "Aimbot On Key", cfg.aimbotOnKey);
    read(j, "Aimbot Key", cfg.aimbotKey);
    read(j, "Aimbot Key Mode", cfg.aimbotKeyMode);

    read(j, "Triggerbot", cfg.triggerbot);
    read(j, "Triggerbot Key", cfg.triggerbotKey);

    read<value_t::object>(j, "Backtrack", cfg.backtrack);
}

//

static void from_json(const json& j, Config::Visuals::Chams::Material& m) {
    from_json(j, static_cast<Color4&>(m));

    read(j, "Enabled", m.enabled);
    read(j, "Health based", m.healthBased);
    read(j, "Blinking", m.blinking);
    read(j, "Wireframe", m.wireframe);
    read(j, "Cover", m.cover);
    read(j, "Ignore-Z", m.ignorez);
    read(j, "Material", m.material);
}

static void from_json(const json& j, Config::Visuals::Chams& c) {
    read_array_opt(j, "Materials", c.materials);
}

static void from_json(const json& j, Config::Visuals::AntiOBS& e) {
    read(j, "Toggle Key", e.toggleKey);
    read(j, "Hold Key", e.holdKey);
    read(j, "Allies", e.allies);
    read(j, "Enemies", e.enemies);
    read(j, "Weapons", e.weapons);
    read(j, "Projectiles", e.projectiles);
    read(j, "Loot Crates", e.lootCrates);
    read(j, "Other Entities", e.otherEntities);
}

static void from_json(const json& j, ImVec2& v) {
    read(j, "X", v.x);
    read(j, "Y", v.y);
}

static void from_json(const json& j, Config::Other::Misc::PurchaseList& pl) {
    read(j, "Enabled", pl.enabled);
    read(j, "Only During Freeze Time", pl.onlyDuringFreezeTime);
    read(j, "Show Prices", pl.showPrices);
    read(j, "No Title Bar", pl.noTitleBar);
    read(j, "Mode", pl.mode);
}

static void from_json(const json& j, Config::Other::Misc::OffscreenEnemies& o) {
    from_json(j, static_cast<ColorToggle&>(o));

    read<value_t::object>(j, "Health Bar", o.healthBar);
}

static void from_json(const json& j, Config::Other::Misc::SpectatorList& sl) {
    read(j, "Enabled", sl.enabled);
    read(j, "No Title Bar", sl.noTitleBar);
    read<value_t::object>(j, "Pos", sl.pos);
    read<value_t::object>(j, "Size", sl.size);
}

static void from_json(const json& j, Config::Other::Misc::Watermark& o) {
    read(j, "Enabled", o.enabled);
}

static void from_json(const json& j, Config::Other::Misc::PreserveKillfeed& o) {
    read(j, "Enabled", o.enabled);
    read(j, "Only Headshots", o.onlyHeadshots);
}

static void from_json(const json& j, Config::Other::Misc& m) {
    read(j, "Menu key", m.menuKey);
    read(j, "Anti AFK kick", m.antiAfkKick);
    read(j, "Auto strafe", m.autoStrafe);
    read(j, "Bunny hop", m.bunnyHop);
    read(j, "Custom clan tag", m.customClanTag);
    read(j, "Clock tag", m.clocktag);
    read(j, "Clan tag", m.clanTag, sizeof(m.clanTag));
    read(j, "Animated clan tag", m.animatedClanTag);
    read(j, "Fast duck", m.fastDuck);
    read(j, "Moonwalk", m.moonwalk);
    read(j, "Edge Jump", m.edgejump);
    read(j, "Edge Jump Key", m.edgejumpkey);
    read(j, "Slowwalk", m.slowwalk);
    read(j, "Slowwalk key", m.slowwalkKey);
    read<value_t::object>(j, "Noscope crosshair", m.noscopeCrosshair);
    read<value_t::object>(j, "Recoil crosshair", m.recoilCrosshair);
    read(j, "Auto pistol", m.autoPistol);
    read(j, "Auto reload", m.autoReload);
    read(j, "Auto accept", m.autoAccept);
    read(j, "Radar hack", m.radarHack);
    read(j, "Reveal ranks", m.revealRanks);
    read(j, "Reveal money", m.revealMoney);
    read(j, "Reveal suspect", m.revealSuspect);
    read(j, "Reveal votes", m.revealVotes);
    read<value_t::object>(j, "Spectator list", m.spectatorList);
    read<value_t::object>(j, "Watermark", m.watermark);
    read<value_t::object>(j, "Offscreen Enemies", m.offscreenEnemies);
    read(j, "Fix animation LOD", m.fixAnimationLOD);
    read(j, "Fix bone matrix", m.fixBoneMatrix);
    read(j, "Fix movement", m.fixMovement);
    read(j, "Disable model occlusion", m.disableModelOcclusion);
    read(j, "Aspect Ratio", m.aspectratio);
    read(j, "Kill message", m.killMessage);
    read<value_t::string>(j, "Kill message string", m.killMessageString);
    read(j, "Name stealer", m.nameStealer);
    read(j, "Disable HUD blur", m.disablePanoramablur);
    read(j, "Ban color", m.banColor);
    read<value_t::string>(j, "Ban text", m.banText);
    read(j, "Fast plant", m.fastPlant);
    read(j, "Fast Stop", m.fastStop);
    read<value_t::object>(j, "Bomb timer", m.bombTimer);
    read(j, "Quick reload", m.quickReload);
    read(j, "Prepare revolver", m.prepareRevolver);
    read(j, "Prepare revolver key", m.prepareRevolverKey);
    read(j, "Hit sound", m.hitSound);
    read(j, "Choked packets", m.chokedPackets);
    read(j, "Choked packets key", m.chokedPacketsKey);
    read(j, "Quick healthshot key", m.quickHealthshotKey);
    read(j, "Grenade predict", m.nadePredict);
    read(j, "Fix tablet signal", m.fixTabletSignal);
    read(j, "Max angle delta", m.maxAngleDelta);
    read(j, "Fix tablet signal", m.fixTabletSignal);
    read<value_t::string>(j, "Custom Hit Sound", m.customHitSound);
    read(j, "Kill sound", m.killSound);
    read<value_t::string>(j, "Custom Kill Sound", m.customKillSound);
    read<value_t::object>(j, "Purchase List", m.purchaseList);
    read<value_t::object>(j, "Reportbot", m.reportbot);
    read(j, "Opposite Hand Knife", m.oppositeHandKnife);
    read<value_t::object>(j, "Preserve Killfeed", m.preserveKillfeed);
}

static void from_json(const json& j, Config::Other::Misc::Reportbot& r) {
    read(j, "Enabled", r.enabled);
    read(j, "Target", r.target);
    read(j, "Delay", r.delay);
    read(j, "Rounds", r.rounds);
    read(j, "Abusive Communications", r.textAbuse);
    read(j, "Griefing", r.griefing);
    read(j, "Wall Hacking", r.wallhack);
    read(j, "Aim Hacking", r.aimbot);
    read(j, "Other Hacking", r.other);
}

static void to_json(json& j, const Config::Other::Misc::Reportbot& o, const Config::Other::Misc::Reportbot& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Target", target);
    WRITE("Delay", delay);
    WRITE("Rounds", rounds);
    WRITE("Abusive Communications", textAbuse);
    WRITE("Griefing", griefing);
    WRITE("Wall Hacking", wallhack);
    WRITE("Aim Hacking", aimbot);
    WRITE("Other Hacking", other);
}

static void to_json(json& j, const Config::Other::Misc::PurchaseList& o, const Config::Other::Misc::PurchaseList& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Only During Freeze Time", onlyDuringFreezeTime);
    WRITE("Show Prices", showPrices);
    WRITE("No Title Bar", noTitleBar);
    WRITE("Mode", mode);
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {}) {
    WRITE("X", x);
    WRITE("Y", y);
}

static void to_json(json& j, const Config::Other::Misc::OffscreenEnemies& o, const Config::Other::Misc::OffscreenEnemies& dummy = {}) {
    to_json(j, static_cast<const ColorToggle&>(o), dummy);

    WRITE("Health Bar", healthBar);
}

static void to_json(json& j, const Config::Other::Misc::SpectatorList& o, const Config::Other::Misc::SpectatorList& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);

    if (const auto window = ImGui::FindWindowByName("Spectator List")) {
        j["Pos"] = window->Pos;
        j["Size"] = window->SizeFull;
    }
}

static void to_json(json& j, const Config::Other::Misc::Watermark& o, const Config::Other::Misc::Watermark& dummy = {}) {
    WRITE("Enabled", enabled);
}

static void to_json(json& j, const Config::Other::Misc::PreserveKillfeed& o, const Config::Other::Misc::PreserveKillfeed& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Only Headshots", onlyHeadshots);
}

static void to_json(json& j, const Config::Other::Misc& o) {
    const Config::Other::Misc dummy;

    WRITE("Menu key", menuKey);
    WRITE("Anti AFK kick", antiAfkKick);
    WRITE("Auto strafe", autoStrafe);
    WRITE("Bunny hop", bunnyHop);
    WRITE("Custom clan tag", customClanTag);
    WRITE("Clock tag", clocktag);

    if (o.clanTag[0])
        j["Clan tag"] = o.clanTag;

    WRITE("Animated clan tag", animatedClanTag);
    WRITE("Fast duck", fastDuck);
    WRITE("Moonwalk", moonwalk);
    WRITE("Edge Jump", edgejump);
    WRITE("Edge Jump Key", edgejumpkey);
    WRITE("Slowwalk", slowwalk);
    WRITE("Slowwalk key", slowwalkKey);
    WRITE("Noscope crosshair", noscopeCrosshair);
    WRITE("Recoil crosshair", recoilCrosshair);
    WRITE("Auto pistol", autoPistol);
    WRITE("Auto reload", autoReload);
    WRITE("Auto accept", autoAccept);
    WRITE("Radar hack", radarHack);
    WRITE("Reveal ranks", revealRanks);
    WRITE("Reveal money", revealMoney);
    WRITE("Reveal suspect", revealSuspect);
    WRITE("Reveal votes", revealVotes);
    WRITE("Spectator list", spectatorList);
    WRITE("Watermark", watermark);
    WRITE("Offscreen Enemies", offscreenEnemies);
    WRITE("Fix animation LOD", fixAnimationLOD);
    WRITE("Fix bone matrix", fixBoneMatrix);
    WRITE("Fix movement", fixMovement);
    WRITE("Disable model occlusion", disableModelOcclusion);
    WRITE("Aspect Ratio", aspectratio);
    WRITE("Kill message", killMessage);
    WRITE("Kill message string", killMessageString);
    WRITE("Name stealer", nameStealer);
    WRITE("Disable HUD blur", disablePanoramablur);
    WRITE("Ban color", banColor);
    WRITE("Ban text", banText);
    WRITE("Fast plant", fastPlant);
    WRITE("Fast Stop", fastStop);
    WRITE("Bomb timer", bombTimer);
    WRITE("Quick reload", quickReload);
    WRITE("Prepare revolver", prepareRevolver);
    WRITE("Prepare revolver key", prepareRevolverKey);
    WRITE("Hit sound", hitSound);
    WRITE("Choked packets", chokedPackets);
    WRITE("Choked packets key", chokedPacketsKey);
    WRITE("Quick healthshot key", quickHealthshotKey);
    WRITE("Grenade predict", nadePredict);
    WRITE("Fix tablet signal", fixTabletSignal);
    WRITE("Max angle delta", maxAngleDelta);
    WRITE("Fix tablet signal", fixTabletSignal);
    WRITE("Custom Hit Sound", customHitSound);
    WRITE("Kill sound", killSound);
    WRITE("Custom Kill Sound", customKillSound);
    WRITE("Purchase List", purchaseList);
    WRITE("Reportbot", reportbot);
    WRITE("Opposite Hand Knife", oppositeHandKnife);
    WRITE("Preserve Killfeed", preserveKillfeed);
}

static void from_json(const json& j, Config::Other::Style& s) {
    read(j, "Menu style", s.menuStyle);
    read(j, "Menu colors", s.menuColors);

    if (j.contains("Colors") && j["Colors"].is_object()) {
        const auto& colors = j["Colors"];

        ImGuiStyle& style = ImGui::GetStyle();

        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (const char* name = ImGui::GetStyleColorName(i); colors.contains(name)) {
                std::array<float, 4> temp;
                read(colors, name, temp);
                style.Colors[i].x = temp[0];
                style.Colors[i].y = temp[1];
                style.Colors[i].z = temp[2];
                style.Colors[i].w = temp[3];
            }
        }
    }
}

void Config::load(size_t id, bool incremental) noexcept {
    load(configs[id].c_str(), incremental);
}

void Config::load(const char8_t* name, bool incremental) noexcept {
    json j;

    if (std::ifstream in { path / name }; in.good()) {
        j = json::parse(in, nullptr, false, true);
        if (j.is_discarded())
            return;
    }
    else {
        return;
    }

    if (!incremental)
        reset();


    /*
    // Legitbot
    read<value_t::object>(j["Legitbot"], "Aimbot", legitbot.aimbot);
    read(j["Legitbot"], "Aimbot On Key", legitbot.aimbotOnKey);
    read(j["Legitbot"], "Aimbot Key", legitbot.aimbotKey);
    read(j["Legitbot"], "Aimbot Key Mode", legitbot.aimbotKeyMode);
    read<value_t::object>(j["Legitbot"], "Triggerbot", legitbot.triggerbot);
    read(j["Legitbot"], "Triggerbot Key", legitbot.triggerbotKey);
    */
    read<value_t::object>(j["Legitbot"], "Backtrack", legitbot.backtrack);
    /*
    read<value_t::object>(j, "Anti Aim", antiAim);

    // Visuals
    read<value_t::object>(j["Visuals"], "Glow", visuals.glow);
    read<value_t::object>(j["Visuals"], "Chams", visuals.chams);
    read<value_t::object>(j["Visuals"], "Anti OBS", visuals.antiOBS);
    read<value_t::object>(j["Visuals"], "Misc", visuals.misc);
    read(j["Visuals"], "Visuals Key", visuals.visualsKey);
    InventoryChanger::fromJson(j["Inventory Changer"]);

    // Other
    read<value_t::object>(j["Other"], "Sound", other.sound);
    read<value_t::object>(j["Other"], "Misc", other.misc);
    read<value_t::object>(j["Other"], "Style", other.style);
     */
}

static void to_json(json& j, const ColorToggleRounding& o, const ColorToggleRounding& dummy = {}) {
    to_json(j, static_cast<const ColorToggle&>(o), dummy);
    WRITE("Rounding", rounding);
}

static void to_json(json& j, const ColorToggleThicknessRounding& o, const ColorToggleThicknessRounding& dummy = {}) {
    to_json(j, static_cast<const ColorToggleRounding&>(o), dummy);
    WRITE("Thickness", thickness);
}

static void to_json(json& j, const Font& o, const Font& dummy = {}) {
    WRITE("Name", name);
}

static void to_json(json& j, const Snapline& o, const Snapline& dummy = {}) {
    to_json(j, static_cast<const ColorToggleThickness&>(o), dummy);
    WRITE("Type", type);
}

static void to_json(json& j, const Box& o, const Box& dummy = {}) {
    to_json(j, static_cast<const ColorToggleRounding&>(o), dummy);
    WRITE("Type", type);
    WRITE("Scale", scale);
    WRITE("Fill", fill);
}

static void to_json(json& j, const Shared& o, const Shared& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Font", font);
    WRITE("Snapline", snapline);
    WRITE("Box", box);
    WRITE("Name", name);
    WRITE("Text Cull Distance", textCullDistance);
}

static void to_json(json& j, const Player& o, const Player& dummy = {}) {
    to_json(j, static_cast<const Shared&>(o), dummy);
    WRITE("Weapon", weapon);
    WRITE("Flash Duration", flashDuration);
    WRITE("Audible Only", audibleOnly);
    WRITE("Spotted Only", spottedOnly);
    WRITE("Health Bar", healthBar);
    WRITE("Skeleton", skeleton);
    WRITE("Head Box", headBox);
}

static void to_json(json& j, const Weapon& o, const Weapon& dummy = {}) {
    to_json(j, static_cast<const Shared&>(o), dummy);
    WRITE("Ammo", ammo);
}

static void to_json(json& j, const Trail& o, const Trail& dummy = {}) {
    to_json(j, static_cast<const ColorToggleThickness&>(o), dummy);
    WRITE("Type", type);
    WRITE("Time", time);
}

static void to_json(json& j, const Trails& o, const Trails& dummy = {}) {
    WRITE("Enabled", enabled);
    WRITE("Local Player", localPlayer);
    WRITE("Allies", allies);
    WRITE("Enemies", enemies);
}

static void to_json(json& j, const Projectile& o, const Projectile& dummy = {}) {
    j = static_cast<const Shared&>(o);

    WRITE("Trails", trails);
}

static void to_json(json& j, const Config::Visuals::Chams::Material& o) {
    const Config::Visuals::Chams::Material dummy;

    to_json(j, static_cast<const Color4&>(o), dummy);
    WRITE("Enabled", enabled);
    WRITE("Health based", healthBased);
    WRITE("Blinking", blinking);
    WRITE("Wireframe", wireframe);
    WRITE("Cover", cover);
    WRITE("Ignore-Z", ignorez);
    WRITE("Material", material);
}

static void to_json(json& j, const Config::Visuals::Chams& o) {
    j["Materials"] = o.materials;
}

static void to_json(json& j, const Config::Visuals::AntiOBS& o, const Config::Visuals::AntiOBS& dummy = {}) {
    WRITE("Toggle Key", toggleKey);
    WRITE("Hold Key", holdKey);
    j["Allies"] = o.allies;
    j["Enemies"] = o.enemies;
    j["Weapons"] = o.weapons;
    j["Projectiles"] = o.projectiles;
    j["Loot Crates"] = o.lootCrates;
    j["Other Entities"] = o.otherEntities;
}

static void to_json(json& j, const ImVec4& o) {
    j[0] = o.x;
    j[1] = o.y;
    j[2] = o.z;
    j[3] = o.w;
}

static void to_json(json& j, const Config::Other::Style& o) {
    const Config::Other::Style dummy;

    WRITE("Menu style", menuStyle);
    WRITE("Menu colors", menuColors);

    auto& colors = j["Colors"];
    ImGuiStyle& style = ImGui::GetStyle();

    for (int i = 0; i < ImGuiCol_COUNT; i++)
        colors[ImGui::GetStyleColorName(i)] = style.Colors[i];
}

void removeEmptyObjects(json& j) noexcept {
    for (auto it = j.begin(); it != j.end();) {
        auto& val = it.value();
        if (val.is_object() || val.is_array())
            removeEmptyObjects(val);
        if (val.empty() && !j.is_array())
            it = j.erase(it);
        else
            ++it;
    }
}

void Config::save(size_t id) const noexcept {
    json j;

    j["Legitbot"] = legitbot.backtrack;

    /*
    j["Legitbot"]["Aimbot On Key"] = legitbot.aimbotOnKey;
    to_json(j["Aimbot Key"], aimbotKey, {});
    j["Aimbot Key mode"] = aimbotKeyMode;

    j["Triggerbot"] = triggerbot;
    to_json(j["Triggerbot Key"], triggerbotHoldKey, {});

    j["Backtrack"] = Backtrack::toJson();
    j["Anti aim"] = AntiAim::toJson();
    j["Glow"] = Glow::toJson();
    j["Chams"] = chams;
    to_json(j["Chams"]["Toggle Key"], chamsToggleKey, {});
    to_json(j["Chams"]["Hold Key"], chamsHoldKey, {});
    j["Sound"] = sound;
    j["Visuals"] = visuals;
    j["Misc"] = misc;
    j["Style"] = style;
    j["Inventory Changer"] = InventoryChanger::toJson();
    */
    removeEmptyObjects(j);

    createConfigDir();
    if (std::ofstream out { path / configs[id] }; out.good())
        out << std::setw(2) << j;
}

void Config::add(const char8_t* name) noexcept {
    if (*name && std::ranges::find(configs, name) == configs.cend()) {
        configs.emplace_back(name);
        save(configs.size() - 1);
    }
}

void Config::remove(size_t id) noexcept {
    std::error_code ec;
    std::filesystem::remove(path / configs[id], ec);
    configs.erase(configs.cbegin() + id);
}

void Config::rename(size_t item, std::u8string_view newName) noexcept {
    std::error_code ec;
    std::filesystem::rename(path / configs[item], path / newName, ec);
    configs[item] = newName;
}

void Config::reset() noexcept {
    legitbot = {}; // FIXME: might be incorrect
    antiAim = {};
    visuals = {};
    InventoryChanger::resetConfig();
    other = {};
}

void Config::listConfigs() noexcept {
    configs.clear();

    std::error_code ec;
    std::transform(std::filesystem::directory_iterator { path, ec },
                   std::filesystem::directory_iterator {},
                   std::back_inserter(configs),
                   [](const auto& entry) { return entry.path().filename().u8string(); });
}

void Config::createConfigDir() const noexcept {
    std::error_code ec;
    std::filesystem::create_directory(path, ec);
}

void Config::openConfigDir() const noexcept {
    createConfigDir();
#ifdef _WIN32
    ShellExecuteW(nullptr, L"open", path.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
    int ret = std::system(("xdg-open " + path.string()).c_str());
#endif
}

void Config::scheduleFontLoad(const std::string& name) noexcept {
    scheduledFonts.push_back(name);
}

#ifdef _WIN32
static auto getFontData(const std::string& fontName) noexcept {
    HFONT font = CreateFontA(0, 0, 0, 0,
                             FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH, fontName.c_str());

    std::unique_ptr<std::byte[]> data;
    DWORD dataSize = GDI_ERROR;

    if (font) {
        HDC hdc = CreateCompatibleDC(nullptr);

        if (hdc) {
            SelectObject(hdc, font);
            dataSize = GetFontData(hdc, 0, 0, nullptr, 0);

            if (dataSize != GDI_ERROR) {
                data = std::make_unique<std::byte[]>(dataSize);
                dataSize = GetFontData(hdc, 0, 0, data.get(), dataSize);

                if (dataSize == GDI_ERROR)
                    data.reset();
            }
            DeleteDC(hdc);
        }
        DeleteObject(font);
    }
    return std::make_pair(std::move(data), dataSize);
}
#endif

bool Config::loadScheduledFonts() noexcept {
    bool result = false;

    for (const auto& fontName : scheduledFonts) {
        if (fontName == "Default") {
            if (fonts.find("Default") == fonts.cend()) {
                ImFontConfig cfg;
                cfg.OversampleH = cfg.OversampleV = 1;
                cfg.PixelSnapH = true;
                cfg.RasterizerMultiply = 1.7f;

                Font newFont;

                cfg.SizePixels = 13.0f;
                newFont.big = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

                cfg.SizePixels = 10.0f;
                newFont.medium = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

                cfg.SizePixels = 8.0f;
                newFont.tiny = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

                fonts.emplace(fontName, newFont);
                result = true;
            }
            continue;
        }

#ifdef _WIN32
        const auto [fontData, fontDataSize] = getFontData(fontName);
        if (fontDataSize == GDI_ERROR)
            continue;

        if (fonts.find(fontName) == fonts.cend()) {
            const auto ranges = Helpers::getFontGlyphRanges();
            ImFontConfig cfg;
            cfg.FontDataOwnedByAtlas = false;
            cfg.RasterizerMultiply = 1.7f;

            Font newFont;
            newFont.tiny = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 8.0f, &cfg, ranges);
            newFont.medium = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 10.0f, &cfg, ranges);
            newFont.big = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 13.0f, &cfg, ranges);
            fonts.emplace(fontName, newFont);
            result = true;
        }
#endif
    }
    scheduledFonts.clear();
    return result;
}
