#include "engineprediction.h"

#include "../utils/interfaces.h"
#include "../utils/memory.h"

#include "../sdk/Entity.h"
#include "../sdk/GameMovement.h"
#include "../sdk/GlobalVars.h"
#include "../sdk/LocalPlayer.h"
#include "../sdk/MoveHelper.h"
#include "../sdk/Prediction.h"

static int localPlayerFlags;

void EnginePrediction::run(UserCmd* cmd) noexcept {
    if (!localPlayer)
        return;

    localPlayerFlags = localPlayer->flags();

    *memory->predictionRandomSeed = 0;

    const auto oldCurrenttime = memory->globalVars->currenttime;
    const auto oldFrametime = memory->globalVars->frametime;

    memory->globalVars->currenttime = memory->globalVars->serverTime();
    memory->globalVars->frametime = memory->globalVars->intervalPerTick;

    memory->moveHelper->setHost(localPlayer.get());
    interfaces->prediction->setupMove(localPlayer.get(), cmd, memory->moveHelper, memory->moveData);
    interfaces->gameMovement->processMovement(localPlayer.get(), memory->moveData);
    interfaces->prediction->finishMove(localPlayer.get(), cmd, memory->moveData);
    memory->moveHelper->setHost(nullptr);

    *memory->predictionRandomSeed = -1;

    memory->globalVars->currenttime = oldCurrenttime;
    memory->globalVars->frametime = oldFrametime;
}

int EnginePrediction::getFlags() noexcept {
    return localPlayerFlags;
}
