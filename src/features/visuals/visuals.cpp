#include <array>
#include <cstring>
#include <string_view>
#include <utility>

#include "visuals.h"

#include "../config/config.h"
#include "../config/configstructs.h"

#include "../../utils/fnv.h"
#include "../../utils/gamedata.h"
#include "../../utils/interfaces.h"
#include "../../utils/memory.h"

#include "../../../lib/sdk/ConVar.h"
#include "../../../lib/sdk/Cvar.h"
#include "../../../lib/sdk/Engine.h"
#include "../../../lib/sdk/Entity.h"
#include "../../../lib/sdk/EntityList.h"
#include "../../../lib/sdk/FrameStage.h"
#include "../../../lib/sdk/GameEvent.h"
#include "../../../lib/sdk/GlobalVars.h"
#include "../../../lib/sdk/Input.h"
#include "../../../lib/sdk/LocalPlayer.h"
#include "../../../lib/sdk/Material.h"
#include "../../../lib/sdk/MaterialSystem.h"
#include "../../../lib/sdk/ViewRenderBeams.h"

/* TODO FIXME: Port to Config.cpp
static void from_json(const json &j, Config::Visuals::Misc::ColorCorrection &c) {
    read(j, "Enabled", c.enabled);
    read(j, "Blue", c.blue);
    read(j, "Red", c.red);
    read(j, "Mono", c.mono);
    read(j, "Saturation", c.saturation);
    read(j, "Ghost", c.ghost);
    read(j, "Green", c.green);
    read(j, "Yellow", c.yellow);
}

static void from_json(const json &j, Config::Visuals::BulletTracers &o) {
    from_json(j, static_cast<ColorToggle &>(o));
}

static void from_json(const json &j, Config::Visuals::Misc &v) {
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

static void to_json(json &j, const Config::Visuals::Misc::ColorCorrection &o, const Config::Visuals::Misc::ColorCorrection &dummy) {
    WRITE("Enabled", enabled);
    WRITE("Blue", blue);
    WRITE("Red", red);
    WRITE("Mono", mono);
    WRITE("Saturation", saturation);
    WRITE("Ghost", ghost);
    WRITE("Green", green);
    WRITE("Yellow", yellow);
}

static void to_json(json &j, const Config::Visuals::BulletTracers &o, const Config::Visuals::BulletTracers &dummy = {}) {
    to_json(j, static_cast<const ColorToggle &>(o), dummy);
}

static void to_json(json &j, const Config::Visuals::Misc &o) {
    const Config::Visuals::Misc dummy;

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
*/

auto& cfg = config->visuals;
bool Visuals::isThirdpersonOn() noexcept { return cfg.misc.thirdperson; }
bool Visuals::isZoomOn() noexcept { return cfg.misc.zoom; }
bool Visuals::isSmokeWireframe() noexcept { return cfg.misc.wireframeSmoke; }
bool Visuals::isDeagleSpinnerOn() noexcept { return cfg.misc.deagleSpinner; }
bool Visuals::shouldRemoveFog() noexcept { return cfg.misc.noFog; }
bool Visuals::shouldRemoveScopeOverlay() noexcept { return cfg.misc.noScopeOverlay; }
bool Visuals::shouldRemoveSmoke() noexcept { return cfg.misc.noSmoke; }
float Visuals::viewModelFov() noexcept { return static_cast<float>(cfg.misc.viewmodelFov); }
float Visuals::fov() noexcept { return static_cast<float>(cfg.misc.fov); }
float Visuals::farZ() noexcept { return static_cast<float>(cfg.misc.farZ); }

void Visuals::performColorCorrection() noexcept {
    if (cfg.misc.colorCorrection.enabled) {
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x49C, 0x908)) = cfg.misc.colorCorrection.blue;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4A4, 0x918)) = cfg.misc.colorCorrection.red;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4AC, 0x928)) = cfg.misc.colorCorrection.mono;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4B4, 0x938)) = cfg.misc.colorCorrection.saturation;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4C4, 0x958)) = cfg.misc.colorCorrection.ghost;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4CC, 0x968)) = cfg.misc.colorCorrection.green;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + WIN32_LINUX(0x4D4, 0x978)) = cfg.misc.colorCorrection.yellow;
    }
}

void Visuals::inverseRagdollGravity() noexcept {
    static auto ragdollGravity = interfaces->cvar->findVar("cl_ragdoll_gravity");
    ragdollGravity->setValue(cfg.misc.inverseRagdollGravity ? -600 : 600);
}

void Visuals::colorWorld() noexcept {
    if (!cfg.misc.world.enabled && !cfg.misc.sky.enabled)
        return;

    for (short h = interfaces->materialSystem->firstMaterial(); h != interfaces->materialSystem->invalidMaterial();
         h = interfaces->materialSystem->nextMaterial(h)) {
        const auto mat = interfaces->materialSystem->getMaterial(h);

        if (!mat || !mat->isPrecached())
            continue;

        const std::string_view textureGroup = mat->getTextureGroupName();

        if (cfg.misc.world.enabled && (textureGroup.starts_with("World") || textureGroup.starts_with("StaticProp"))) {
            if (cfg.misc.world.asColor3().rainbow)
                mat->colorModulate(rainbowColor(cfg.misc.world.asColor3().rainbowSpeed));
            else
                mat->colorModulate(cfg.misc.world.asColor3().color);
        }
        else if (cfg.misc.sky.enabled && textureGroup.starts_with("SkyBox")) {
            if (cfg.misc.sky.asColor3().rainbow)
                mat->colorModulate(rainbowColor(cfg.misc.sky.asColor3().rainbowSpeed));
            else
                mat->colorModulate(cfg.misc.sky.asColor3().color);
        }
    }
}

void Visuals::modifySmoke(FrameStage stage) noexcept {
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr std::array smokeMaterials {
        "particle/vistasmokev1/vistasmokev1_emods",
        "particle/vistasmokev1/vistasmokev1_emods_impactdust",
        "particle/vistasmokev1/vistasmokev1_fire",
        "particle/vistasmokev1/vistasmokev1_smokegrenade"
    };

    for (const auto mat : smokeMaterials) {
        const auto material = interfaces->materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && cfg.misc.noSmoke);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, stage == FrameStage::RENDER_START && cfg.misc.wireframeSmoke);
    }
}

void Visuals::thirdperson() noexcept {
    if (!cfg.misc.thirdperson)
        return;

    memory->input->isCameraInThirdPerson =
        (!cfg.misc.thirdpersonKey.isSet() || cfg.misc.thirdpersonKey.isToggled()) && localPlayer && localPlayer->isAlive();
    memory->input->cameraOffset.z = static_cast<float>(cfg.misc.thirdpersonDistance);
}

void Visuals::removeVisualRecoil(FrameStage stage) noexcept {
    if (!localPlayer || !localPlayer->isAlive())
        return;

    static Vector aimPunch;
    static Vector viewPunch;

    if (stage == FrameStage::RENDER_START) {
        aimPunch = localPlayer->aimPunchAngle();
        viewPunch = localPlayer->viewPunchAngle();

        if (cfg.misc.noAimPunch)
            localPlayer->aimPunchAngle() = Vector {};

        if (cfg.misc.noViewPunch)
            localPlayer->viewPunchAngle() = Vector {};

    }
    else if (stage == FrameStage::RENDER_END) {
        localPlayer->aimPunchAngle() = aimPunch;
        localPlayer->viewPunchAngle() = viewPunch;
    }
}

void Visuals::removeBlur(FrameStage stage) noexcept {
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static auto blur = interfaces->materialSystem->findMaterial("dev/scope_bluroverlay");
    blur->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && cfg.misc.noBlur);
}

void Visuals::updateBrightness() noexcept {
    static auto brightness = interfaces->cvar->findVar("mat_force_tonemap_scale");
    brightness->setValue(cfg.misc.brightness);
}

void Visuals::removeGrass(FrameStage stage) noexcept {
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
        interfaces->materialSystem->findMaterial(grassMaterialName)
        ->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && cfg.misc.noGrass);
}

void Visuals::remove3dSky() noexcept {
    static auto sky = interfaces->cvar->findVar("r_3dsky");
    sky->setValue(!cfg.misc.no3dSky);
}

void Visuals::removeShadows() noexcept {
    static auto shadows = interfaces->cvar->findVar("cl_csm_enabled");
    shadows->setValue(!cfg.misc.noShadows);
}

void Visuals::applyZoom(FrameStage stage) noexcept {
    if (cfg.misc.zoom && localPlayer) {
        if (stage == FrameStage::RENDER_START && (localPlayer->fov() == 90 || localPlayer->fovStart() == 90)) {
            if (cfg.misc.zoomKey.isToggled()) {
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

void Visuals::applyScreenEffects() noexcept {
    if (!cfg.misc.screenEffect)
        return;

    const auto material = interfaces->materialSystem->findMaterial([] {
        constexpr std::array effects {
            "effects/dronecam",
            "effects/underwater_overlay",
            "effects/healthboost",
            "effects/dangerzone_screen"
        };

        if (cfg.misc.screenEffect <= 2 || static_cast<std::size_t>(cfg.misc.screenEffect - 2) >= effects.size())
            return effects[0];
        return effects[cfg.misc.screenEffect - 2];
    }());

    if (cfg.misc.screenEffect == 1)
        material->findVar("$c0_x")->setValue(0.0f);
    else if (cfg.misc.screenEffect == 2)
        material->findVar("$c0_x")->setValue(0.1f);
    else if (cfg.misc.screenEffect >= 4)
        material->findVar("$c0_x")->setValue(1.0f);

    DRAW_SCREEN_EFFECT(material)
}

void Visuals::hitEffect(GameEvent* event) noexcept {
    if (cfg.misc.hitEffect && localPlayer) {
        static float lastHitTime = 0.0f;

        if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
            lastHitTime = memory->globalVars->realtime;
            return;
        }

        if (lastHitTime + cfg.misc.hitEffectTime >= memory->globalVars->realtime) {
            constexpr auto getEffectMaterial = [] {
                static constexpr const char* effects[] {
                    "effects/dronecam",
                    "effects/underwater_overlay",
                    "effects/healthboost",
                    "effects/dangerzone_screen"
                };

                if (cfg.misc.hitEffect <= 2)
                    return effects[0];
                return effects[cfg.misc.hitEffect - 2];
            };

            auto material = interfaces->materialSystem->findMaterial(getEffectMaterial());
            if (cfg.misc.hitEffect == 1)
                material->findVar("$c0_x")->setValue(0.0f);
            else if (cfg.misc.hitEffect == 2)
                material->findVar("$c0_x")->setValue(0.1f);
            else if (cfg.misc.hitEffect >= 4)
                material->findVar("$c0_x")->setValue(1.0f);

            DRAW_SCREEN_EFFECT(material)
        }
    }
}

void Visuals::hitMarker(GameEvent* event, ImDrawList* drawList) noexcept {
    if (cfg.misc.hitMarker == 0)
        return;

    static float lastHitTime = 0.0f;

    if (event) {
        if (localPlayer && event->getInt("attacker") == localPlayer->getUserId())
            lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + cfg.misc.hitMarkerTime < memory->globalVars->realtime)
        return;

    switch (cfg.misc.hitMarker) {
    case 1:const auto & mid = ImGui::GetIO().DisplaySize / 2.0f;
        constexpr auto color = IM_COL32(255, 255, 255, 255);
        drawList->AddLine({ mid.x - 10, mid.y - 10 }, { mid.x - 4, mid.y - 4 }, color);
        drawList->AddLine({ mid.x + 10.5f, mid.y - 10.5f }, { mid.x + 4.5f, mid.y - 4.5f }, color);
        drawList->AddLine({ mid.x + 10.5f, mid.y + 10.5f }, { mid.x + 4.5f, mid.y + 4.5f }, color);
        drawList->AddLine({ mid.x - 10, mid.y + 10 }, { mid.x - 4, mid.y + 4 }, color);
        break;
    }
}

void Visuals::disablePostProcessing(FrameStage stage) noexcept {
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    *memory->disablePostProcessing = stage == FrameStage::RENDER_START && cfg.misc.disablePostProcessing;
}

void Visuals::reduceFlashEffect() noexcept {
    if (localPlayer)
        localPlayer->flashMaxAlpha() = 255.0f - cfg.misc.flashReduction * 2.55f;
}

bool Visuals::removeHands(const char* modelName) noexcept {
    return cfg.misc.noHands && std::strstr(modelName, "arms") && !std::strstr(modelName, "sleeve");
}

bool Visuals::removeSleeves(const char* modelName) noexcept {
    return cfg.misc.noSleeves && std::strstr(modelName, "sleeve");
}

bool Visuals::removeWeapons(const char* modelName) noexcept {
    return cfg.misc.noWeapons && std::strstr(modelName, "models/weapons/v_")
        && !std::strstr(modelName, "arms") && !std::strstr(modelName, "tablet")
        && !std::strstr(modelName, "parachute") && !std::strstr(modelName, "fists");
}

void Visuals::skybox(FrameStage stage) noexcept {
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    if (stage == FrameStage::RENDER_START && cfg.misc.skybox > 0 && static_cast<std::size_t>(cfg.misc.skybox) < skyboxList.size()) {
        memory->loadSky(skyboxList[cfg.misc.skybox]);
    }
    else {
        static const auto sv_skyname = interfaces->cvar->findVar("sv_skyname");
        memory->loadSky(sv_skyname->string);
    }
}

void Visuals::bulletTracer(GameEvent& event) noexcept {
    if (!cfg.misc.bulletTracers.enabled)
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
    }
    else {
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

    beamInfo.red = 255.0f * cfg.misc.bulletTracers.asColor4().color[0];
    beamInfo.green = 255.0f * cfg.misc.bulletTracers.asColor4().color[1];
    beamInfo.blue = 255.0f * cfg.misc.bulletTracers.asColor4().color[2];
    beamInfo.brightness = 255.0f * cfg.misc.bulletTracers.asColor4().color[3];

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

void Visuals::drawMolotovHull(ImDrawList* drawList) noexcept {
    if (!cfg.misc.molotovHull.enabled)
        return;

    const auto color = Helpers::calculateColor(cfg.misc.molotovHull.asColor4());

    GameData::Lock lock;

    static const auto flameCircumference = [] {
        std::array<Vector, 72> points {};
        for (std::size_t i = 0; i < points.size(); ++i) {
            constexpr auto flameRadius =
                60.0f; // https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/server/cstrike15/Effects/inferno.cpp#L889
            points[i] = Vector { flameRadius * std::cos(Helpers::deg2rad(i * (360.0f / points.size()))),
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

            std::swap(screenPoints[0],
                      *std::min_element(screenPoints.begin(),
                          screenPoints.begin() + count,
                          [](const auto& a, const auto& b) { return a.y < b.y || (a.y == b.y && a.x < b.x); }));

            constexpr auto orientation = [](const ImVec2& a, const ImVec2& b, const ImVec2& c) {
                return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
            };

            std::sort(screenPoints.begin() + 1,
                      screenPoints.begin() + count,
                      [&](const auto& a, const auto& b) { return orientation(screenPoints[0], a, b) > 0.0f; });
            drawList->AddConvexPolyFilled(screenPoints.data(), count, color);
        }
    }
}

void Visuals::updateEventListeners(bool forceRemove) noexcept {
    class ImpactEventListener : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) override { bulletTracer(*event); }
    };

    static ImpactEventListener listener;
    static bool listenerRegistered = false;

    if (cfg.misc.bulletTracers.enabled && !listenerRegistered) {
        interfaces->gameEventManager->addListener(&listener, "bullet_impact");
        listenerRegistered = true;
    }
    else if ((!cfg.misc.bulletTracers.enabled || forceRemove) && listenerRegistered) {
        interfaces->gameEventManager->removeListener(&listener);
        listenerRegistered = false;
    }
}

void Visuals::updateInput() noexcept {
    cfg.visualsKey.handleToggle();
    cfg.misc.thirdpersonKey.handleToggle();
    cfg.misc.zoomKey.handleToggle();
}

