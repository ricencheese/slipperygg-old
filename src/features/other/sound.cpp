#include <array>
#include <string_view>

#include "sound.h"

#include "../config/config.h"
#include "../config/configstructs.h"

#include "../../utils/interfaces.h"

#include "../../../lib/sdk/Entity.h"
#include "../../../lib/sdk/EntityList.h"
#include "../../../lib/sdk/LocalPlayer.h"

#if OSIRIS_SOUND()

void Sound::modulateSound(std::string_view name, int entityIndex, float& volume) noexcept {
    auto modulateVolume = [&](int Config::Other::Sound::Player::* proj) {
        if (const auto entity = interfaces->entityList->getEntity(entityIndex); localPlayer && entity && entity->isPlayer()) {
            if (entityIndex == localPlayer->index())
                volume *= std::invoke(proj, config->other.sound.players[0]) / 100.0f;
            else if (!entity->isOtherEnemy(localPlayer.get()))
                volume *= std::invoke(proj, config->other.sound.players[1]) / 100.0f;
            else
                volume *= std::invoke(proj, config->other.sound.players[2]) / 100.0f;
        }
    };

    modulateVolume(&Config::Other::Sound::Player::masterVolume);

    using namespace std::literals;

    if (name == "Player.DamageHelmetFeedback"sv)
        modulateVolume(&Config::Other::Sound::Player::headshotVolume);
    else if (name.find("Weapon"sv) != std::string_view::npos && name.find("Single"sv) != std::string_view::npos)
        modulateVolume(&Config::Other::Sound::Player::weaponVolume);
    else if (name.find("Step"sv) != std::string_view::npos)
        modulateVolume(&Config::Other::Sound::Player::footstepVolume);
    else if (name.find("Chicken"sv) != std::string_view::npos)
        volume *= config->other.sound.chickenVolume / 100.0f;
}


// TODO: Implement this into Config.cpp
/*
static void to_json(json &j, const Config::Other::Sound::Player &o) {
    const Config::Other::Sound::Player dummy;

    WRITE("Master Volume", masterVolume);
    WRITE("Headshot Volume", headshotVolume);
    WRITE("Weapon Volume", weaponVolume);
    WRITE("Footstep Volume", footstepVolume);
}

json Sound::toJson() noexcept {
    const config->other.sound dummy;

    json j;
    to_json(j["Chicken Volume"], soundConfig.chickenVolume, dummy.chickenVolume);
    j["Players"] = soundConfig.players;
    return j;
}

static void from_json(const json &j, Config::Other::Sound::Player &p) {
    read(j, "Master Volume", p.masterVolume);
    read(j, "Headshot Volume", p.headshotVolume);
    read(j, "Weapon Volume", p.weaponVolume);
    read(j, "Footstep Volume", p.footstepVolume);
}

void Sound::fromJson(const json &j) noexcept {
    read(j, "Chicken Volume", soundConfig.chickenVolume);
    read(j, "Players", soundConfig.players);
}
*/

#else
void Sound::modulateSound(std::string_view name, int entityIndex, float& volume) noexcept {}
#endif
