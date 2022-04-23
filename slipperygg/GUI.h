#pragma once

#include <memory>
#include "Hacks/Misc.h"
struct ImFont;

class GUI {
public:
    GUI() noexcept;
    void render() noexcept;
    void handleToggle() noexcept;
    [[nodiscard]] bool isOpen() const noexcept { return open; }
private:
    bool open = true;

    void updateColors() const noexcept;
    void renderMenuBar() noexcept;
    void renderHomeWindow(bool contentOnly = false) noexcept;
    void renderAimbotWindow(bool contentOnly = false) noexcept;
    void renderTriggerbotWindow(bool contentOnly = false) noexcept;
    void renderChamsWindow(bool contentOnly = false) noexcept;
    void renderStyleWindow(bool contentOnly = false) noexcept;
    //void renderConfigWindow(bool contentOnly = false) noexcept;
    void renderGuiStyle2() noexcept;
    void renderMenuBarStyle3() noexcept;
    void renderGuiStyle3() noexcept;

    struct {
        bool home = false;
        bool aimbot = false;
        bool triggerbot = false;
        bool chams = false;
        bool sound = false;
        bool style = false;
        bool config = false;
        bool configPopup = false;
        bool shouldDrawNewMenu = false;
    } window;

    struct {
        ImFont* normal15px = nullptr;
        ImFont* backgroundCubes = nullptr;
        ImFont* icons = nullptr;
    } fonts;
    std::string steamName{Misc::getSteamName()};
    float timeToNextConfigRefresh = 0.1f;
};

inline std::unique_ptr<GUI> gui;
