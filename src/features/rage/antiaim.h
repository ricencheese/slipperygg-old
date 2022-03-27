#pragma once

#include "json/jsonforward.h"

struct UserCmd;
struct Vector;

#define OSIRIS_ANTIAIM() true

namespace AntiAim {
    void run(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept;
}
