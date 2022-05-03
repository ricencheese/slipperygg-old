#include <array>
#include <cstring>
#include <string_view>
#include <utility>
#include <vector>

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../GUI.h"
#include "../ConfigStructs.h"
#include "../fnv.h"
#include "../GameData.h"
#include "../Helpers.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../imguiCustom.h"
#include "Visuals.h"
#include "Glow.h"

#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Input.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"
#include "../SDK/ViewRenderBeams.h"
#include "../SDK/ClientMode.h"

struct BulletTracers : ColorToggle {
    BulletTracers() : ColorToggle{ 0.0f, 0.75f, 1.0f, 1.0f } {}
};

struct VisualsConfig {
    bool worldModulationWinOpen{ false };
    bool disablePostProcessing{ false };
    bool inverseRagdollGravity{ false };
    bool noFog{ false };
    bool no3dSky{ false };
    bool noAimPunch{ false };
    bool noViewPunch{ false };
    bool noHands{ false };
    bool noSleeves{ false };
    bool noWeapons{ false };
    bool noSmoke{ false };
    bool noBlur{ false };
    bool noScopeOverlay{ false };
    bool noGrass{ false };
    bool noShadows{ false };
    bool wireframeSmoke{ false };
    bool zoom{ false };
    KeyBindToggle zoomKey;
    bool thirdperson{ false };
    KeyBindToggle thirdpersonKey;
    int thirdpersonDistance{ 0 };
    int viewmodelFov{ 0 };
    int fov{ 0 };
    int farZ{ 0 };
    int flashReduction{ 0 };
    float brightness{ 0.0f };
    int skybox{ 0 };
    ColorToggle3 world;
    ColorToggle3 sky;
    bool deagleSpinner{ false };
    int screenEffect{ 0 };
    int hitEffect{ 0 };
    float hitEffectTime{ 0.6f };
    int hitMarker{ 0 };
    float hitMarkerTime{ 0.6f };
    BulletTracers bulletTracers;
    ColorToggle molotovHull{ 1.0f, 0.27f, 0.0f, 0.3f };
    bool viewmodelModulation{ false };
    int viewodelFOV{ 68 };
    float viewmodelOffsetX{ 0 };
    float viewmodelOffsetY{ 0 };
    float viewmodelOffsetZ{ 0 };
    bool useOldWeaponBob{ false };
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
    bool isSecondPage{ false };
} visualsConfig;


static void from_json(const json& j, VisualsConfig::ColorCorrection& c)
{
    read(j, "Enabled", c.enabled);
    read(j, "Blue", c.blue);
    read(j, "Red", c.red);
    read(j, "Mono", c.mono);
    read(j, "Saturation", c.saturation);
    read(j, "Ghost", c.ghost);
    read(j, "Green", c.green);
    read(j, "Yellow", c.yellow);
}

static void from_json(const json& j, BulletTracers& o)
{
    from_json(j, static_cast<ColorToggle&>(o));
}

static void from_json(const json& j, VisualsConfig& v)
{
    read(j, "Disable post-processing", v.disablePostProcessing);
    read(j, "Inverse ragdoll gravity", v.inverseRagdollGravity);
    read(j, "No fog", v.noFog);
    read(j, "No 3d sky", v.no3dSky);
    read(j, "No aim punch", v.noAimPunch);
    read(j, "No view punch", v.noViewPunch);
    read(j, "No hands", v.noHands);
    read(j, "No sleeves", v.noSleeves);
    read(j, "No weapons", v.noWeapons);
    read(j, "No smoke", v.noSmoke);
    read(j, "No blur", v.noBlur);
    read(j, "No scope overlay", v.noScopeOverlay);
    read(j, "No grass", v.noGrass);
    read(j, "No shadows", v.noShadows);
    read(j, "Wireframe smoke", v.wireframeSmoke);
    read(j, "Zoom", v.zoom);
    read(j, "Zoom key", v.zoomKey);
    read(j, "Thirdperson", v.thirdperson);
    read(j, "Thirdperson key", v.thirdpersonKey);
    read(j, "Thirdperson distance", v.thirdpersonDistance);
    read(j, "Viewmodel FOV", v.viewmodelFov);
    read(j, "Viewmodel Offset X", v.viewmodelOffsetX);
    read(j, "Viewmodel Offset Y", v.viewmodelOffsetY);
    read(j, "Viewmodel Offset Z", v.viewmodelOffsetZ);
    read(j, "FOV", v.fov);
    read(j, "Far Z", v.farZ);
    read(j, "Flash reduction", v.flashReduction);
    read(j, "Brightness", v.brightness);
    read(j, "Skybox", v.skybox);
    read<value_t::object>(j, "World", v.world);
    read<value_t::object>(j, "Sky", v.sky);
    read(j, "Deagle spinner", v.deagleSpinner);
    read(j, "Screen effect", v.screenEffect);
    read(j, "Hit effect", v.hitEffect);
    read(j, "Hit effect time", v.hitEffectTime);
    read(j, "Hit marker", v.hitMarker);
    read(j, "Hit marker time", v.hitMarkerTime);
    read<value_t::object>(j, "Color correction", v.colorCorrection);
    read<value_t::object>(j, "Bullet Tracers", v.bulletTracers);
    read<value_t::object>(j, "Molotov Hull", v.molotovHull);
}

static void to_json(json& j, const VisualsConfig::ColorCorrection& o, const VisualsConfig::ColorCorrection& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Blue", blue);
    WRITE("Red", red);
    WRITE("Mono", mono);
    WRITE("Saturation", saturation);
    WRITE("Ghost", ghost);
    WRITE("Green", green);
    WRITE("Yellow", yellow);
}

static void to_json(json& j, const BulletTracers& o, const BulletTracers& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);
}

static void to_json(json& j, const VisualsConfig& o)
{
    const VisualsConfig dummy;

    WRITE("Disable post-processing", disablePostProcessing);
    WRITE("Inverse ragdoll gravity", inverseRagdollGravity);
    WRITE("No fog", noFog);
    WRITE("No 3d sky", no3dSky);
    WRITE("No aim punch", noAimPunch);
    WRITE("No view punch", noViewPunch);
    WRITE("No hands", noHands);
    WRITE("No sleeves", noSleeves);
    WRITE("No weapons", noWeapons);
    WRITE("No smoke", noSmoke);
    WRITE("No blur", noBlur);
    WRITE("No scope overlay", noScopeOverlay);
    WRITE("No grass", noGrass);
    WRITE("No shadows", noShadows);
    WRITE("Wireframe smoke", wireframeSmoke);
    WRITE("Zoom", zoom);
    WRITE("Zoom key", zoomKey);
    WRITE("Thirdperson", thirdperson);
    WRITE("Thirdperson key", thirdpersonKey);
    WRITE("Thirdperson distance", thirdpersonDistance);
    WRITE("Viewmodel FOV", viewmodelFov);
    WRITE("Viewmodel Offset X", viewmodelOffsetX);
    WRITE("Viewmodel Offset Y", viewmodelOffsetY);
    WRITE("Viewmodel Offset Z", viewmodelOffsetZ);
    WRITE("FOV", fov);
    WRITE("Far Z", farZ);
    WRITE("Flash reduction", flashReduction);
    WRITE("Brightness", brightness);
    WRITE("Skybox", skybox);
    WRITE("World", world);
    WRITE("Sky", sky);
    WRITE("Deagle spinner", deagleSpinner);
    WRITE("Screen effect", screenEffect);
    WRITE("Hit effect", hitEffect);
    WRITE("Hit effect time", hitEffectTime);
    WRITE("Hit marker", hitMarker);
    WRITE("Hit marker time", hitMarkerTime);
    WRITE("Color correction", colorCorrection);
    WRITE("Bullet Tracers", bulletTracers);
    WRITE("Molotov Hull", molotovHull);
}

bool Visuals::isThirdpersonOn() noexcept
{
    return visualsConfig.thirdperson;
}

bool Visuals::isZoomOn() noexcept
{
    return visualsConfig.zoom;
}

bool Visuals::isSmokeWireframe() noexcept
{
    return visualsConfig.wireframeSmoke;
}

bool Visuals::isDeagleSpinnerOn() noexcept
{
    return visualsConfig.deagleSpinner;
}

bool Visuals::shouldRemoveFog() noexcept
{
    return visualsConfig.noFog;
}

bool Visuals::shouldRemoveScopeOverlay() noexcept
{
    return visualsConfig.noScopeOverlay;
}

bool Visuals::shouldRemoveSmoke() noexcept
{
    return visualsConfig.noSmoke;
}

float Visuals::viewModelFov() noexcept
{
    return static_cast<float>(visualsConfig.viewmodelFov);
}

float Visuals::fov() noexcept
{
    return static_cast<float>(visualsConfig.fov);
}

float Visuals::farZ() noexcept
{
    return static_cast<float>(visualsConfig.farZ);
}

void Visuals::performColorCorrection() noexcept
{
    if (const auto& cfg = visualsConfig.colorCorrection; cfg.enabled) {
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x49C, 0x908)) = cfg.blue;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4A4, 0x918)) = cfg.red;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4AC, 0x928)) = cfg.mono;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4B4, 0x938)) = cfg.saturation;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4C4, 0x958)) = cfg.ghost;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4CC, 0x968)) = cfg.green;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4D4, 0x978)) = cfg.yellow;
    }
}

void Visuals::inverseRagdollGravity() noexcept
{
    static auto ragdollGravity = interfaces->cvar->findVar("cl_ragdoll_gravity");
    ragdollGravity->setValue(visualsConfig.inverseRagdollGravity ? -600 : 600);
}

void Visuals::colorWorld() noexcept
{
    if (!visualsConfig.world.enabled && !visualsConfig.sky.enabled)
        return;

    for (short h = interfaces->materialSystem->firstMaterial(); h != interfaces->materialSystem->invalidMaterial(); h = interfaces->materialSystem->nextMaterial(h)) {
        const auto mat = interfaces->materialSystem->getMaterial(h);

        if (!mat || !mat->isPrecached())
            continue;

        const std::string_view textureGroup = mat->getTextureGroupName();

        if (visualsConfig.world.enabled && (textureGroup.starts_with("World") || textureGroup.starts_with("StaticProp"))) {
            if (visualsConfig.world.asColor3().rainbow)
                mat->colorModulate(rainbowColor(visualsConfig.world.asColor3().rainbowSpeed));
            else
                mat->colorModulate(visualsConfig.world.asColor3().color);
        } else if (visualsConfig.sky.enabled && textureGroup.starts_with("SkyBox")) {
            if (visualsConfig.sky.asColor3().rainbow)
                mat->colorModulate(rainbowColor(visualsConfig.sky.asColor3().rainbowSpeed));
            else
                mat->colorModulate(visualsConfig.sky.asColor3().color);
        }
    }
}

void Visuals::modifySmoke(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr std::array smokeMaterials{
        "particle/vistasmokev1/vistasmokev1_emods",
        "particle/vistasmokev1/vistasmokev1_emods_impactdust",
        "particle/vistasmokev1/vistasmokev1_fire",
        "particle/vistasmokev1/vistasmokev1_smokegrenade"
    };

    for (const auto mat : smokeMaterials) {
        const auto material = interfaces->materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && visualsConfig.noSmoke);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, stage == FrameStage::RENDER_START && visualsConfig.wireframeSmoke);
    }
}

void Visuals::thirdperson() noexcept
{
    if (!visualsConfig.thirdperson)
        return;

    memory->input->isCameraInThirdPerson = (!visualsConfig.thirdpersonKey.isSet() || visualsConfig.thirdpersonKey.isToggled()) && localPlayer && localPlayer->isAlive();
    memory->input->cameraOffset.z = static_cast<float>(visualsConfig.thirdpersonDistance); 
}

void Visuals::removeVisualRecoil(FrameStage stage) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    static Vector aimPunch;
    static Vector viewPunch;

    if (stage == FrameStage::RENDER_START) {
        aimPunch = localPlayer->aimPunchAngle();
        viewPunch = localPlayer->viewPunchAngle();

        if (visualsConfig.noAimPunch)
            localPlayer->aimPunchAngle() = Vector{ };

        if (visualsConfig.noViewPunch)
            localPlayer->viewPunchAngle() = Vector{ };

    } else if (stage == FrameStage::RENDER_END) {
        localPlayer->aimPunchAngle() = aimPunch;
        localPlayer->viewPunchAngle() = viewPunch;
    }
}

void Visuals::removeBlur(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static auto blur = interfaces->materialSystem->findMaterial("dev/scope_bluroverlay");
    blur->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && visualsConfig.noBlur);
}

void Visuals::updateBrightness() noexcept
{
    static auto brightness = interfaces->cvar->findVar("mat_force_tonemap_scale");
    brightness->setValue(visualsConfig.brightness);
}

void Visuals::removeGrass(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr auto getGrassMaterialName = []() noexcept -> const char* {
        switch (fnv::hashRuntime(interfaces->engine->getLevelName())) {
        case fnv::hash("dz_blacksite"): return "detail/detailsprites_survival";
        case fnv::hash("dz_sirocco"): return "detail/dust_massive_detail_sprites";
        case fnv::hash("coop_autumn"): return "detail/autumn_detail_sprites";
        case fnv::hash("dz_frostbite"): return "ski/detail/detailsprites_overgrown_ski";
        // dz_junglety has been removed in 7/23/2020 patch
        // case fnv::hash("dz_junglety"): return "detail/tropical_grass";
        default: return nullptr;
        }
    };

    if (const auto grassMaterialName = getGrassMaterialName())
        interfaces->materialSystem->findMaterial(grassMaterialName)->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && visualsConfig.noGrass);
}

void Visuals::remove3dSky() noexcept
{
    static auto sky = interfaces->cvar->findVar("r_3dsky");
    sky->setValue(!visualsConfig.no3dSky);
}

void Visuals::removeShadows() noexcept
{
    static auto shadows = interfaces->cvar->findVar("cl_csm_enabled");
    shadows->setValue(!visualsConfig.noShadows);
}

void Visuals::applyZoom(FrameStage stage) noexcept
{
    if (visualsConfig.zoom && localPlayer) {
        if (stage == FrameStage::RENDER_START && (localPlayer->fov() == 90 || localPlayer->fovStart() == 90)) {
            if (visualsConfig.zoomKey.isToggled()) {
                localPlayer->fov() = 40;
                localPlayer->fovStart() = 40;
            }
        }
    }
}

#ifdef _WIN32
#undef xor
#define DRAW_SCREEN_EFFECT(material) \
{ \
    const auto drawFunction = memory->drawScreenEffectMaterial; \
    int w, h; \
    interfaces->engine->getScreenSize(w, h); \
    __asm { \
        __asm push h \
        __asm push w \
        __asm push 0 \
        __asm xor edx, edx \
        __asm mov ecx, material \
        __asm call drawFunction \
        __asm add esp, 12 \
    } \
}

#else
#define DRAW_SCREEN_EFFECT(material) \
{ \
    int w, h; \
    interfaces->engine->getScreenSize(w, h); \
    reinterpret_cast<void(*)(Material*, int, int, int, int)>(memory->drawScreenEffectMaterial)(material, 0, 0, w, h); \
}
#endif

void Visuals::applyScreenEffects() noexcept
{
    if (!visualsConfig.screenEffect)
        return;

    const auto material = interfaces->materialSystem->findMaterial([] {
        constexpr std::array effects{
            "effects/dronecam",
            "effects/underwater_overlay",
            "effects/healthboost",
            "effects/dangerzone_screen"
        };

        if (visualsConfig.screenEffect <= 2 || static_cast<std::size_t>(visualsConfig.screenEffect - 2) >= effects.size())
            return effects[0];
        return effects[visualsConfig.screenEffect - 2];
    }());

    if (visualsConfig.screenEffect == 1)
        material->findVar("$c0_x")->setValue(0.0f);
    else if (visualsConfig.screenEffect == 2)
        material->findVar("$c0_x")->setValue(0.1f);
    else if (visualsConfig.screenEffect >= 4)
        material->findVar("$c0_x")->setValue(1.0f);

    DRAW_SCREEN_EFFECT(material)
}

void Visuals::hitEffect(GameEvent* event) noexcept
{
    if (visualsConfig.hitEffect && localPlayer) {
        static float lastHitTime = 0.0f;

        if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
            lastHitTime = memory->globalVars->realtime;
            return;
        }

        if (lastHitTime + visualsConfig.hitEffectTime >= memory->globalVars->realtime) {
            constexpr auto getEffectMaterial = [] {
                static constexpr const char* effects[]{
                "effects/dronecam",
                "effects/underwater_overlay",
                "effects/healthboost",
                "effects/dangerzone_screen"
                };

                if (visualsConfig.hitEffect <= 2)
                    return effects[0];
                return effects[visualsConfig.hitEffect - 2];
            };

           
            auto material = interfaces->materialSystem->findMaterial(getEffectMaterial());
            if (visualsConfig.hitEffect == 1)
                material->findVar("$c0_x")->setValue(0.0f);
            else if (visualsConfig.hitEffect == 2)
                material->findVar("$c0_x")->setValue(0.1f);
            else if (visualsConfig.hitEffect >= 4)
                material->findVar("$c0_x")->setValue(1.0f);

            DRAW_SCREEN_EFFECT(material)
        }
    }
}

void Visuals::hitMarker(GameEvent* event, ImDrawList* drawList) noexcept
{
    if (visualsConfig.hitMarker == 0)
        return;
    static float lastHitTime = 0.0f;

    if (event) {
        if (localPlayer && event->getInt("attacker") == localPlayer->getUserId())
            lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + visualsConfig.hitMarkerTime < memory->globalVars->realtime)
        return;

    switch (visualsConfig.hitMarker) {
    case 1:
        const auto& mid = ImGui::GetIO().DisplaySize / 2.0f;
        constexpr auto color = IM_COL32(255, 255, 255, 255);
        drawList->AddLine({mid.x - 10, mid.y - 10}, {mid.x - 4, mid.y - 4}, color);
        drawList->AddLine({ mid.x + 10.5f, mid.y - 10.5f }, { mid.x + 4.5f, mid.y - 4.5f }, color);
        drawList->AddLine({ mid.x + 10.5f, mid.y + 10.5f }, { mid.x + 4.5f, mid.y + 4.5f }, color);
        drawList->AddLine({ mid.x - 10, mid.y + 10 }, { mid.x - 4, mid.y + 4 }, color);
        break;
    }
}


void Visuals::disablePostProcessing(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    *memory->disablePostProcessing = stage == FrameStage::RENDER_START && visualsConfig.disablePostProcessing;
}

void Visuals::reduceFlashEffect() noexcept
{
    if (localPlayer)
        localPlayer->flashMaxAlpha() = 255.0f - visualsConfig.flashReduction * 2.55f;
}

bool Visuals::removeHands(const char* modelName) noexcept
{
    return visualsConfig.noHands && std::strstr(modelName, "arms") && !std::strstr(modelName, "sleeve");
}

bool Visuals::removeSleeves(const char* modelName) noexcept
{
    return visualsConfig.noSleeves && std::strstr(modelName, "sleeve");
}

bool Visuals::removeWeapons(const char* modelName) noexcept
{
    return visualsConfig.noWeapons && std::strstr(modelName, "models/weapons/v_")
        && !std::strstr(modelName, "arms") && !std::strstr(modelName, "tablet")
        && !std::strstr(modelName, "parachute") && !std::strstr(modelName, "fists");
}

void Visuals::skybox(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    if (stage == FrameStage::RENDER_START && visualsConfig.skybox > 0 && static_cast<std::size_t>(visualsConfig.skybox) < skyboxList.size()) {
        memory->loadSky(skyboxList[visualsConfig.skybox]);
    } else {
        static const auto sv_skyname = interfaces->cvar->findVar("sv_skyname");
        memory->loadSky(sv_skyname->string);
    }
}

void Visuals::bulletTracer(GameEvent& event) noexcept
{
    if (!visualsConfig.bulletTracers.enabled)
        return;

    if (!localPlayer)
        return;

    if (event.getInt("userid") != localPlayer->getUserId())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    BeamInfo beamInfo;

    if (!localPlayer->shouldDraw()) {
        const auto viewModel = interfaces->entityList->getEntityFromHandle(localPlayer->viewModel());
        if (!viewModel)
            return;

        if (!viewModel->getAttachment(activeWeapon->getMuzzleAttachmentIndex1stPerson(viewModel), beamInfo.start))
            return;
    } else {
        const auto worldModel = interfaces->entityList->getEntityFromHandle(activeWeapon->weaponWorldModel());
        if (!worldModel)
            return;

        if (!worldModel->getAttachment(activeWeapon->getMuzzleAttachmentIndex3rdPerson(), beamInfo.start))
            return;
    }

    beamInfo.end.x = event.getFloat("x");
    beamInfo.end.y = event.getFloat("y");
    beamInfo.end.z = event.getFloat("z");

    beamInfo.modelName = "sprites/physbeam.vmt";
    beamInfo.modelIndex = -1;
    beamInfo.haloName = nullptr;
    beamInfo.haloIndex = -1;

    beamInfo.red = 255.0f * visualsConfig.bulletTracers.asColor4().color[0];
    beamInfo.green = 255.0f * visualsConfig.bulletTracers.asColor4().color[1];
    beamInfo.blue = 255.0f * visualsConfig.bulletTracers.asColor4().color[2];
    beamInfo.brightness = 255.0f * visualsConfig.bulletTracers.asColor4().color[3];

    beamInfo.type = 0;
    beamInfo.life = 0.0f;
    beamInfo.amplitude = 0.0f;
    beamInfo.segments = -1;
    beamInfo.renderable = true;
    beamInfo.speed = 0.2f;
    beamInfo.startFrame = 0;
    beamInfo.frameRate = 0.0f;
    beamInfo.width = 2.0f;
    beamInfo.endWidth = 2.0f;
    beamInfo.flags = 0x40;
    beamInfo.fadeLength = 20.0f;

    if (const auto beam = memory->viewRenderBeams->createBeamPoints(beamInfo)) {
        constexpr auto FBEAM_FOREVER = 0x4000;
        beam->flags &= ~FBEAM_FOREVER;
        beam->die = memory->globalVars->currenttime + 2.0f;
    }
}

void Visuals::drawMolotovHull(ImDrawList* drawList) noexcept
{
    if (!visualsConfig.molotovHull.enabled)
        return;

    const auto color = Helpers::calculateColor(visualsConfig.molotovHull.asColor4());

    GameData::Lock lock;

    static const auto flameCircumference = [] {
        std::array<Vector, 72> points;
        for (std::size_t i = 0; i < points.size(); ++i) {
            constexpr auto flameRadius = 60.0f; // https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/server/cstrike15/Effects/inferno.cpp#L889
            points[i] = Vector{ flameRadius * std::cos(Helpers::deg2rad(i * (360.0f / points.size()))),
                                flameRadius * std::sin(Helpers::deg2rad(i * (360.0f / points.size()))),
                                0.0f };
        }
        return points;
    }();

    for (const auto& molotov : GameData::infernos()) {
        for (const auto& pos : molotov.points) {
            std::array<ImVec2, flameCircumference.size()> screenPoints;
            std::size_t count = 0;

            for (const auto& point : flameCircumference) {
                if (Helpers::worldToScreen(pos + point, screenPoints[count]))
                    ++count;
            }

            if (count < 1)
                continue;

            std::swap(screenPoints[0], *std::min_element(screenPoints.begin(), screenPoints.begin() + count, [](const auto& a, const auto& b) { return a.y < b.y || (a.y == b.y && a.x < b.x); }));

            constexpr auto orientation = [](const ImVec2& a, const ImVec2& b, const ImVec2& c) {
                return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
            };

            std::sort(screenPoints.begin() + 1, screenPoints.begin() + count, [&](const auto& a, const auto& b) { return orientation(screenPoints[0], a, b) > 0.0f; });
            drawList->AddConvexPolyFilled(screenPoints.data(), count, color);
        }
    }
}

void Visuals::updateEventListeners(bool forceRemove) noexcept
{
    class ImpactEventListener : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) override { bulletTracer(*event); }
    };

    static ImpactEventListener listener;
    static bool listenerRegistered = false;

    if (visualsConfig.bulletTracers.enabled && !listenerRegistered) {
        interfaces->gameEventManager->addListener(&listener, "bullet_impact");
        listenerRegistered = true;
    } else if ((!visualsConfig.bulletTracers.enabled || forceRemove) && listenerRegistered) {
        interfaces->gameEventManager->removeListener(&listener);
        listenerRegistered = false;
    }
}

void Visuals::updateInput() noexcept
{
    visualsConfig.thirdpersonKey.handleToggle();
    visualsConfig.zoomKey.handleToggle();
}

static bool windowOpen = false;

void Visuals::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Visuals")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Visuals");
        ImGui::SetWindowPos("Visuals", { 100.0f, 100.0f });
    }
}

void Visuals::useOldWeaponBob() noexcept
{
    if (!visualsConfig.useOldWeaponBob)
        return;

    static const auto cl_use_new_headbob = interfaces->cvar->findVar("cl_use_new_headbob");
    
    cl_use_new_headbob->setValue(0);
}

void Visuals::tabItem() noexcept
{
    if (ImGui::BeginTabItem("Visuals")) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Visuals::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 680.0f, 450.0f });
        ImGui::Begin("Visuals", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    //    if (ImGui::Button(("World modulation"), ImVec2(150.f, 20.f))) {
     //       ImGui::OpenPopup("World modulation");
      //  }

    ImGui::SetNextWindowBgAlpha(0.4f);
    ImGui::BeginChild(1, ImVec2(391, 378), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (!visualsConfig.isSecondPage)
    {    ImGui::PushItemWidth(290.0f);
    if (ImGui::BeginCombo("", "World Modulation")) {
        ImGui::Selectable("Disable post-processing", &visualsConfig.disablePostProcessing, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("Inverse ragdoll gravity", &visualsConfig.inverseRagdollGravity, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No fog", &visualsConfig.noFog, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No 3d skybox", &visualsConfig.no3dSky, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No aimpunch", &visualsConfig.noAimPunch, ImGuiSelectableFlags_DontClosePopups);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("head go up when you shoot");
            ImGui::EndTooltip();
        }
        ImGui::Selectable("No viewpunch", &visualsConfig.noViewPunch, ImGuiSelectableFlags_DontClosePopups);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("head go up when you get shot");
            ImGui::EndTooltip();
        }
        ImGui::Selectable("No hands", &visualsConfig.noHands, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No sleeves", &visualsConfig.noSleeves, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No weapon", &visualsConfig.noWeapons, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No smoke", &visualsConfig.noSmoke, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No blur", &visualsConfig.noBlur, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No scope overlay", &visualsConfig.noScopeOverlay, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No grass", &visualsConfig.noGrass, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("No shadows", &visualsConfig.noShadows, ImGuiSelectableFlags_DontClosePopups);
        ImGui::Selectable("Wireframe smoke", &visualsConfig.wireframeSmoke, ImGuiSelectableFlags_DontClosePopups);
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    ImGui::Checkbox("Thirdperson", &visualsConfig.thirdperson);
    ImGui::SameLine();
    ImGui::PushID("Thirdperson Key");
    ImGui::hotkey("", visualsConfig.thirdpersonKey);
    ImGui::PopID();
    ImGui::PushItemWidth(290.0f);
    ImGui::PushID(0);
    ImGui::PopID();
    ImGui::PushID(1);
    ImGui::Checkbox("Viewmodel Modulation", &visualsConfig.viewmodelModulation);
    ImGui::SameLine();
    ImGui::PushID("Viewmodel Modulation");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");
    if (ImGui::BeginPopup("")) {
        if (ImGui::SliderInt("Viewmodel FOV", &visualsConfig.viewmodelFov, 0, 100)) {      //viewmodel modulation code, standart value of fov = 68
            int viewmodelFov = visualsConfig.viewmodelFov;
            static auto viewmodelfov = interfaces->cvar->findVar("viewmodel_fov");
            viewmodelfov->setValue(viewmodelFov);
        }
        if (ImGui::SliderFloat("Viewmodel offset X", &visualsConfig.viewmodelOffsetX, -2, 2.5)) {
            int viewmodeloffsetX = visualsConfig.viewmodelOffsetX;
            static auto viewmodeloffsetx = interfaces->cvar->findVar("viewmodel_offset_x");
            viewmodeloffsetx->setValue(viewmodeloffsetX);
        }
        if (ImGui::SliderFloat("Viewmodel offset Y", &visualsConfig.viewmodelOffsetY, -2, 2)) {
            int viewmodeloffsetY = visualsConfig.viewmodelOffsetY;
            static auto viewmodeloffsety = interfaces->cvar->findVar("viewmodel_offset_y");
            viewmodeloffsety->setValue(viewmodeloffsetY);
        }if (ImGui::SliderFloat("Viewmodel offset Z", &visualsConfig.viewmodelOffsetZ, -2, 2)) {
            int viewmodeloffsetZ = visualsConfig.viewmodelOffsetZ;
            static auto viewmodeloffsetz = interfaces->cvar->findVar("viewmodel_offset_z");
            viewmodeloffsetz->setValue(viewmodeloffsetZ);
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PopID();
    ImGui::PushID(3);
    ImGui::SliderInt("", &visualsConfig.farZ, 0, 2000, "Far Z: %d");
    ImGui::PopID();
    ImGui::PushID(5);
    ImGui::SliderFloat("", &visualsConfig.brightness, 0.0f, 1.0f, "Brightness: %.2f");
    ImGui::PopID();

    ImGui::Checkbox("Color correction", &visualsConfig.colorCorrection.enabled);
    ImGui::SameLine();
    if (bool ccPopup = ImGui::Button("Edit"))
        ImGui::OpenPopup("##popup");

    if (ImGui::BeginPopup("##popup")) {
        ImGui::VSliderFloat("##1", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.blue, 0.0f, 1.0f, "Blue\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##2", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.red, 0.0f, 1.0f, "Red\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##3", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.mono, 0.0f, 1.0f, "Mono\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##4", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.saturation, 0.0f, 1.0f, "Sat\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##5", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.ghost, 0.0f, 1.0f, "Ghost\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##6", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.green, 0.0f, 1.0f, "Green\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##7", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.yellow, 0.0f, 1.0f, "Yellow\n%.3f"); ImGui::SameLine();
        ImGui::EndPopup();
    }
        ImGuiCustom::colorPicker("Sky color", visualsConfig.sky);
        ImGui::Checkbox("Deagle spinner", &visualsConfig.deagleSpinner);
        ImGui::Combo("Hit effect", &visualsConfig.hitEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
        ImGui::SliderFloat("Hit effect time", &visualsConfig.hitEffectTime, 0.1f, 1.5f, "%.2fs");
        ImGui::Combo("Hit marker", &visualsConfig.hitMarker, "None\0Default (Cross)\0");
        ImGui::SliderFloat("Hit marker time", &visualsConfig.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
    }

    if (visualsConfig.isSecondPage)
    {

        ImGuiCustom::colorPicker("Bullet Tracers", visualsConfig.bulletTracers.asColor4().color.data(), &visualsConfig.bulletTracers.asColor4().color[3], nullptr, nullptr, &visualsConfig.bulletTracers.enabled);
        ImGuiCustom::colorPicker("Molotov Hull", visualsConfig.molotovHull);
        ImGui::PushItemWidth(290);
        ImGui::Combo("Screen effect", &visualsConfig.screenEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
        ImGuiCustom::colorPicker("World color", visualsConfig.world);
        ImGui::PushItemWidth(290);
        ImGui::Combo("Skybox", &visualsConfig.skybox, Visuals::skyboxList.data(), Visuals::skyboxList.size());
        ImGui::PopItemWidth();
        ImGui::PushID(4);
        ImGui::SliderInt("", &visualsConfig.flashReduction, 0, 100, "Flash reduction: %d%%");
        ImGui::PopID();
        ImGui::PushID(2);
        ImGui::SliderInt("", &visualsConfig.fov, -60, 60, "FOV: %d");
        ImGui::PopID();
        ImGui::Checkbox("Zoom", &visualsConfig.zoom);
        ImGui::SameLine();
        ImGui::PushID("Zoom Key");
        ImGui::hotkey("", visualsConfig.zoomKey);
        ImGui::PopID();
        ImGui::SliderInt("", &visualsConfig.thirdpersonDistance, 0, 1000, "Thirdperson distance: %d");
       // Misc::drawMiscVisuals();
    }
    ImGui::SetCursorPos(ImVec2(0, 336));
    if (ImGui::Button("Next page", ImVec2(391, 30))) {
        (visualsConfig.isSecondPage) = (!visualsConfig.isSecondPage);
    };
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::SetNextWindowBgAlpha(0.4f);
        ImGui::BeginChild(3, ImVec2(391, 378), true);
        ImGui::Text("Glow"); ImGui::Separator();
        Glow::drawGUI(true);
        ImGui::EndChild();
    if (!contentOnly)
        ImGui::End();
}

json Visuals::toJson() noexcept
{
    json j;
    to_json(j, visualsConfig);
    return j;
}

void Visuals::fromJson(const json& j) noexcept
{
    from_json(j, visualsConfig);
}

void Visuals::resetConfig() noexcept
{
    visualsConfig = {};
}
