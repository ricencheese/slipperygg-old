#pragma once

#include <string_view>

#include "../../../lib/json/jsonforward.h"

#define OSIRIS_SOUND() true

namespace Sound {
    void modulateSound(std::string_view name, int entityIndex, float& volume) noexcept;
}
