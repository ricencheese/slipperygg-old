#pragma once

#include <string>
#include "singleton.hpp"
#include "imgui/imgui.h"

struct IDirect3DDevice9;

namespace Misc
{
    void updateClanTag(bool = false) noexcept;
}

class Menu
    : public Singleton<Menu>
{
public:
    void Initialize();
    void Shutdown();

    void OnDeviceLost();
    void OnDeviceReset();

    void Render();

    void Toggle();

    void LightTheme();

    bool IsVisible() const { return _visible; }

private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;
};