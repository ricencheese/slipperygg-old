#pragma once

#include <memory>

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
    void renderAimbotWindow() noexcept;
    void renderTriggerbotWindow() noexcept;
    void renderBacktrackWindow() noexcept;
    void renderAntiAimWindow() noexcept;
    void renderGlowWindow() noexcept;
    void renderChamsWindow() noexcept;
    void renderAntiOBSWindow() noexcept;
    void renderVisualsWindow() noexcept;
    void renderSoundWindow() noexcept;
    void renderMiscWindow() noexcept;
    void renderStyleWindow() noexcept;
    void renderConfigWindow() noexcept;
    
    struct {
        ImFont* normal15px = nullptr;
    } fonts;
    
    float timeToNextConfigRefresh = 5.0f; // seconds
};

inline std::unique_ptr<GUI> gui;
