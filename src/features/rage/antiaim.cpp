#include "AntiAim.h"
#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"

#if OSIRIS_ANTIAIM()

void AntiAim::run(UserCmd* cmd, const Vector &previousViewAngles, const Vector &currentViewAngles, bool &sendPacket) noexcept {
    /*
    if (config->antiAim.enabled) {
        if (!localPlayer)
            return;
        
        if (config->antiAim.pitch && cmd->viewangles.x == currentViewAngles.x)
            cmd->viewangles.x = config->antiAim.pitchAngle;
        
        if (config->antiAim.yaw && !sendPacket && cmd->viewangles.y == currentViewAngles.y) {
            cmd->viewangles.y += localPlayer->getMaxDesyncAngle();
            if (std::abs(cmd->sidemove) < 5.0f) {
                if (cmd->buttons & UserCmd::IN_DUCK)
                    cmd->sidemove = cmd->tickCount & 1 ? 3.25f : -3.25f;
                else
                    cmd->sidemove = cmd->tickCount & 1 ? 1.1f : -1.1f;
            }
        }
    }
     */
}

#else

namespace AntiAim
{
    void run(UserCmd*, const Vector&, const Vector&, bool&) noexcept {}
}

#endif
