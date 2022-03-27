#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "configstructs.h"

#include "../../utils/inpututil.h"

struct ImFont;

class Config {
public:
    Config() noexcept;
    void load(std::size_t, bool incremental) noexcept;
    void load(const char8_t* name, bool incremental) noexcept;
    void save(std::size_t) const noexcept;
    void add(const char8_t*) noexcept;
    void remove(std::size_t) noexcept;
    void rename(std::size_t, std::u8string_view newName) noexcept;
    void reset() noexcept;
    void listConfigs() noexcept;
    void createConfigDir() const noexcept;
    void openConfigDir() const noexcept;

    constexpr auto& getConfigs() noexcept {
        return configs;
    }

    struct Font {
        ImFont* tiny;
        ImFont* medium;
        ImFont* big;
    };

    // Legitbot

    struct Legitbot {
        struct Aimbot {
            bool enabled { false };
            bool aimlock { false };
            bool silent { false };
            bool friendlyFire { false };
            bool visibleOnly { true };
            bool scopedOnly { true };
            bool ignoreFlash { false };
            bool ignoreSmoke { false };
            bool autoShot { false };
            bool autoScope { false };
            float fov { 0.0f };
            float smooth { 1.0f };
            int bone { 0 };
            float maxAimInaccuracy { 1.0f };
            float maxShotInaccuracy { 1.0f };
            int minDamage { 1 };
            bool killshot { false };
            bool betweenShots { true };
        } aimbotStruct;
        std::array<Aimbot, 40> aimbot;
        bool aimbotOnKey { false };
        KeyBind aimbotKey;
        int aimbotKeyMode { 0 };

        struct Triggerbot {
            bool enabled = false;
            bool friendlyFire = false;
            bool scopedOnly = true;
            bool ignoreFlash = false;
            bool ignoreSmoke = false;
            bool killshot = false;
            int hitgroup = 0;
            int shotDelay = 0;
            int minDamage = 1;
            float burstTime = 0.0f;
        } triggerbotStruct;
        std::array<Triggerbot, 40> triggerbot;
        KeyBind triggerbotKey;

        struct Backtrack {
            bool enabled = false;
            bool ignoreSmoke = false;
            bool recoilBasedFov = false;
            int timeLimit = 200;
        } backtrack;
    } legitbot;

    // Anti Aim

    struct AntiAim {
        bool test = false;
    } antiAim;

    // Visuals

    struct Visuals {
        KeyBindToggle visualsKey;

        struct BulletTracers : ColorToggle { BulletTracers() : ColorToggle { 0.0f, 0.75f, 1.0f, 1.0f } {} };

        struct GlowItem : Color4 {
            bool enabled = false;
            bool healthBased = false;
            int style = 0;
        };

        struct PlayerGlow { GlowItem all, visible, occluded; };

        std::unordered_map<std::string, PlayerGlow> playerGlow;
        std::unordered_map<std::string, GlowItem> glow;
        std::vector<std::pair<int, int>> customGlowEntities;

        struct Chams {
            struct Material : Color4 {
                bool enabled = false;
                bool healthBased = false;
                bool blinking = false;
                bool wireframe = false;
                bool cover = false;
                bool ignorez = false;
                int material = 0;
            };
            std::array<Material, 7> materials;
        } chamsStruct;
        std::unordered_map<std::string, Chams> chams;

        struct AntiOBS {
            KeyBindToggle toggleKey;
            KeyBind holdKey;

            std::unordered_map<std::string, Player> allies;
            std::unordered_map<std::string, Player> enemies;
            std::unordered_map<std::string, Weapon> weapons;
            std::unordered_map<std::string, Projectile> projectiles;
            std::unordered_map<std::string, Shared> lootCrates;
            std::unordered_map<std::string, Shared> otherEntities;
        } antiOBS;

        struct Misc {
            bool disablePostProcessing { false };
            bool inverseRagdollGravity { false };
            bool noFog { false };
            bool no3dSky { false };
            bool noAimPunch { false };
            bool noViewPunch { false };
            bool noHands { false };
            bool noSleeves { false };
            bool noWeapons { false };
            bool noSmoke { false };
            bool noBlur { false };
            bool noScopeOverlay { false };
            bool noGrass { false };
            bool noShadows { false };
            bool wireframeSmoke { false };
            bool zoom { false };
            KeyBindToggle zoomKey;
            bool thirdperson { false };
            KeyBindToggle thirdpersonKey;
            int thirdpersonDistance { 0 };
            int viewmodelFov { 0 };
            int fov { 0 };
            int farZ { 0 };
            int flashReduction { 0 };
            float brightness { 0.0f };
            int skybox { 0 };
            ColorToggle3 world;
            ColorToggle3 sky;
            bool deagleSpinner { false };
            int screenEffect { 0 };
            int hitEffect { 0 };
            float hitEffectTime { 0.6f };
            int hitMarker { 0 };
            float hitMarkerTime { 0.6f };
            BulletTracers bulletTracers;
            ColorToggle molotovHull { 1.0f, 0.27f, 0.0f, 0.3f };
            bool viewmodelModulation { false };
            int viewodelFOV { 68 };
            float viewmodelOffsetX { 0 };
            float viewmodelOffsetY { 0 };
            float viewmodelOffsetZ { 0 };

            struct ColorCorrection {
                bool enabled = false;
                float blue = 0.0f;
                float red = 0.0f;
                float mono = 0.0f;
                float saturation = 0.0f;
                float ghost = 0.0f;
                float green = 0.0f;
                float yellow = 0.0f;
            } colorCorrection;
        } misc;
    } visuals;

    // Other

    struct Other {
        struct Sound {
            int chickenVolume = 100;

            struct Player {
                int masterVolume = 100;
                int headshotVolume = 100;
                int weaponVolume = 100;
                int footstepVolume = 100;
            };

            std::array<Player, 3> players;
        } sound;

        struct Misc {
            Misc() { clanTag[0] = '\0'; }

            struct PreserveKillfeed {
                bool enabled = false;
                bool onlyHeadshots = false;
            };

            struct OffscreenEnemies : ColorToggle {
                OffscreenEnemies() : ColorToggle { 1.0f, 0.26f, 0.21f, 1.0f } {}
                HealthBar healthBar;
            };

            struct PurchaseList {
                bool enabled = false;
                bool onlyDuringFreezeTime = false;
                bool showPrices = false;
                bool noTitleBar = false;

                enum Mode {
                    Details = 0,
                    Summary
                };
                int mode = Details;
            };

            KeyBind menuKey { KeyBind::INSERT };
            bool antiAfkKick { false };
            bool autoStrafe { false };
            bool bunnyHop { false };
            bool customClanTag { false };
            bool clocktag { false };
            bool animatedClanTag { false };
            bool fastDuck { false };
            bool moonwalk { false };
            bool edgejump { false };
            bool slowwalk { false };
            bool autoPistol { false };
            bool autoReload { false };
            bool autoAccept { false };
            bool radarHack { false };
            bool revealRanks { false };
            bool revealMoney { false };
            bool revealSuspect { false };
            bool revealVotes { false };
            bool fixAnimationLOD { false };
            bool fixBoneMatrix { false };
            bool fixMovement { false };
            bool disableModelOcclusion { false };
            bool nameStealer { false };
            bool disablePanoramablur { false };
            bool killMessage { false };
            bool nadePredict { false };
            bool fixTabletSignal { false };
            bool fastPlant { false };
            bool fastStop { false };
            bool quickReload { false };
            bool prepareRevolver { false };
            bool oppositeHandKnife = false;
            PreserveKillfeed preserveKillfeed;
            char clanTag[16] {};
            int tagAnimationType { 0 };
            KeyBind edgejumpkey;
            KeyBind slowwalkKey { KeyBind::LSHIFT };
            ColorToggleThickness noscopeCrosshair;
            ColorToggleThickness recoilCrosshair;

            struct SpectatorList {
                bool enabled { false };
                bool noTitleBar { false };
                ImVec2 pos { 0, 0 };
                ImVec2 size { 200.0f, 200.0f };
            };
            SpectatorList spectatorList;

            struct Watermark {
                bool enabled = true;
            };
            Watermark watermark;
            float aspectratio { 0 };
            std::string killMessageString { "slipped right past you there" };
            int banColor { 6 };
            std::string banText { "Cheater has been permanently banned from official CS:GO servers." };
            ColorToggle3 bombTimer { 1.0f, 0.55f, 0.0f };
            KeyBind prepareRevolverKey;
            int hitSound { 0 };
            int chokedPackets { 0 };
            KeyBind chokedPacketsKey;
            int quickHealthshotKey { 0 };
            float maxAngleDelta { 255.0f };
            int killSound { 0 };
            std::string customKillSound;
            std::string customHitSound;
            PurchaseList purchaseList;

            struct Reportbot {
                bool enabled = false;
                bool textAbuse = false;
                bool griefing = false;
                bool wallhack = true;
                bool aimbot = true;
                bool other = true;
                int target = 0;
                int delay = 1;
                int rounds = 1;
            } reportbot;

            OffscreenEnemies offscreenEnemies;
        } misc;

        struct Style {
            int menuStyle { 0 };
            int menuColors { 0 };
#ifdef _WIN32 // FIXME: Temporary fix until OpenGL and Vulkan texture rendering for Linux
            int menuBackground { 0 }; // I don't think we should use the cheat background (It looks out-of-place)
#else
            bool menuBackground { false };
#endif
            ColorToggle menuBackgroundColor;
        } style;
    } other;

    void scheduleFontLoad(const std::string& name) noexcept;
    bool loadScheduledFonts() noexcept;
    [[nodiscard]] const auto& getSystemFonts() const noexcept { return systemFonts; }
    [[nodiscard]] const auto& getFonts() const noexcept { return fonts; }
private:
    std::vector<std::string> scheduledFonts { "Default" };
    std::vector<std::string> systemFonts { "Default" };
    std::unordered_map<std::string, Font> fonts;
    std::filesystem::path path;
    std::vector<std::u8string> configs;
};

inline std::unique_ptr<Config> config;
