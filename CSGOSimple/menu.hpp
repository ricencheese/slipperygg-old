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
    void ConfigWindow();
    void Toggle();
    void ToggleCfg(); //this is fucky but I don't know how to do it any other way yet :)


    //void LightTheme();

    bool IsVisible() const { return _visible; }

    bool IsVisiblecfg() const { return _visiblecfg;  }

private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;
    bool              _visiblecfg;
};