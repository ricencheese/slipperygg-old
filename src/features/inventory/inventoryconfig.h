#pragma once

#include "../../lib/json/jsonforward.h"

namespace InventoryChanger
{
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
