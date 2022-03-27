#include <algorithm>
#include <string>

#include "glow.h"

#include "../config/config.h"

#include "../../utils/helpers.h"
#include "../../utils/interfaces.h"
#include "../../utils/memory.h"

#include "../../../lib/sdk/ClientClass.h"
#include "../../../lib/sdk/Engine.h"
#include "../../../lib/sdk/Entity.h"
#include "../../../lib/sdk/EntityList.h"
#include "../../../lib/sdk/GlowObjectManager.h"
#include "../../../lib/sdk/LocalPlayer.h"

#if OSIRIS_GLOW()

void Glow::render() noexcept {
    if (!localPlayer)
        return;

    Glow::clearCustomObjects();

    if (config->visuals.visualsKey.isSet() && !config->visuals.visualsKey.isToggled())
        return;

    const auto highestEntityIndex = interfaces->entityList->getHighestEntityIndex();
    for (int i = interfaces->engine->getMaxClients() + 1; i <= highestEntityIndex; ++i) {
        const auto entity = interfaces->entityList->getEntity(i);
        if (!entity || entity->isDormant())
            continue;

        switch (entity->getClientClass()->classId) {
        case ClassId::EconEntity:
        case ClassId::BaseCSGrenadeProjectile:
        case ClassId::BreachChargeProjectile:
        case ClassId::BumpMineProjectile:
        case ClassId::DecoyProjectile:
        case ClassId::MolotovProjectile:
        case ClassId::SensorGrenadeProjectile:
        case ClassId::SmokeGrenadeProjectile:
        case ClassId::SnowballProjectile:
        case ClassId::Hostage:
        case ClassId::CSRagdoll:
            if (!memory->glowObjectManager->hasGlowEffect(entity)) {
                if (auto index { memory->glowObjectManager->registerGlowObject(entity) }; index != -1)
                    config->visuals.customGlowEntities.emplace_back(i, index);
            }
            break;
        default:break;
        }
    }

    for (int i = 0; i < memory->glowObjectManager->glowObjectDefinitions.size; i++) {
        GlowObjectDefinition& glowobject = memory->glowObjectManager->glowObjectDefinitions[i];

        auto entity = glowobject.entity;

        if (glowobject.isUnused() || !entity || entity->isDormant())
            continue;

        auto applyGlow = [&glowobject](const Config::Visuals::GlowItem& glow, int health = 0) noexcept {
            if (glow.enabled) {
                glowobject.renderWhenOccluded = true;
                glowobject.glowAlpha = glow.color[3];
                glowobject.glowStyle = glow.style;
                glowobject.glowAlphaMax = 0.6f;
                if (glow.healthBased && health) {
                    Helpers::healthColor(std::clamp(health / 100.0f, 0.0f, 1.0f), glowobject.glowColor.x, glowobject.glowColor.y, glowobject.glowColor.z);
                }
                else if (glow.rainbow) {
                    const auto [r, g, b] {rainbowColor(glow.rainbowSpeed)};
                    glowobject.glowColor = { r, g, b };
                }
                else {
                    glowobject.glowColor = { glow.color[0], glow.color[1], glow.color[2] };
                }
            }
        };

        auto applyPlayerGlow = [applyGlow](const std::string& name, Entity* entity) noexcept {
            if (config->visuals.playerGlow[name].all.enabled)
                applyGlow(config->visuals.playerGlow[name].all, entity->health());
            else if (config->visuals.playerGlow[name].visible.enabled && entity->visibleTo(localPlayer.get()))
                applyGlow(config->visuals.playerGlow[name].visible, entity->health());
            else if (config->visuals.playerGlow[name].occluded.enabled && !entity->visibleTo(localPlayer.get()))
                applyGlow(config->visuals.playerGlow[name].occluded, entity->health());
        };

        switch (entity->getClientClass()->classId) {
        case ClassId::CSPlayer:
            if (!entity->isAlive())
                break;
            if (auto activeWeapon { entity->getActiveWeapon() }; activeWeapon && activeWeapon->getClientClass()->classId == ClassId::C4
                && activeWeapon->c4StartedArming())
                applyPlayerGlow("Planting", entity);
            else if (entity->isDefusing())
                applyPlayerGlow("Defusing", entity);
            else if (entity == localPlayer.get())
                applyGlow(config->visuals.glow["Local Player"], entity->health());
            else if (entity->isOtherEnemy(localPlayer.get()))
                applyPlayerGlow("Enemies", entity);
            else
                applyPlayerGlow("Allies", entity);
            break;
        case ClassId::C4: applyGlow(config->visuals.glow["C4"]);
            break;
        case ClassId::PlantedC4: applyGlow(config->visuals.glow["Planted C4"]);
            break;
        case ClassId::Chicken: applyGlow(config->visuals.glow["Chickens"]);
            break;
        case ClassId::EconEntity: applyGlow(config->visuals.glow["Defuse Kits"]);
            break;

        case ClassId::BaseCSGrenadeProjectile:
        case ClassId::BreachChargeProjectile:
        case ClassId::BumpMineProjectile:
        case ClassId::DecoyProjectile:
        case ClassId::MolotovProjectile:
        case ClassId::SensorGrenadeProjectile:
        case ClassId::SmokeGrenadeProjectile:
        case ClassId::SnowballProjectile:applyGlow(config->visuals.glow["Projectiles"]);
            break;

        case ClassId::Hostage: applyGlow(config->visuals.glow["Hostages"]);
            break;
        case ClassId::CSRagdoll: applyGlow(config->visuals.glow["Ragdolls"]);
            break;
        default:
            if (entity->isWeapon()) {
                applyGlow(config->visuals.glow["Weapons"]);
                if (!config->visuals.glow["Weapons"].enabled)
                    glowobject.renderWhenOccluded = false;
            }
        }
    }
}

void Glow::clearCustomObjects() noexcept {
    for (const auto& [entityIndex, glowObjectIndex] : config->visuals.customGlowEntities)
        memory->glowObjectManager->unregisterGlowObject(glowObjectIndex);

    config->visuals.customGlowEntities.clear();
}

/* TODO:
static void to_json(json &j, const GlowItem &o, const GlowItem &dummy = {}) {
    to_json(j, static_cast<const Color4 &>(o), dummy);
    WRITE("Enabled", enabled);
    WRITE("Health based", healthBased);
    WRITE("Style", style);
}

static void to_json(json &j, const PlayerGlow &o, const PlayerGlow &dummy = {}) {
    WRITE("All", all);
    WRITE("Visible", visible);
    WRITE("Occluded", occluded);
}

json Glow::toJson() noexcept {
    json j;
    j["Items"] = glowConfig;
    j["Players"] = playerGlowConfig;
    to_json(j["Toggle Key"], glowToggleKey, {});
    to_json(j["Hold Key"], glowHoldKey, {});
    return j;
}

static void from_json(const json &j, GlowItem &g) {
    from_json(j, static_cast<Color4 &>(g));

    read(j, "Enabled", g.enabled);
    read(j, "Health based", g.healthBased);
    read(j, "Style", g.style);
}

static void from_json(const json &j, PlayerGlow &g) {
    read<value_t::object>(j, "All", g.all);
    read<value_t::object>(j, "Visible", g.visible);
    read<value_t::object>(j, "Occluded", g.occluded);
}

void Glow::fromJson(const json &j) noexcept {
    read(j, "Items", glowConfig);
    read(j, "Players", playerGlowConfig);
    read(j, "Toggle Key", glowToggleKey);
    read(j, "Hold Key", glowHoldKey);
}
*/

#else

void Glow::render() noexcept {}
void Glow::clearCustomObjects() noexcept {}

#endif
