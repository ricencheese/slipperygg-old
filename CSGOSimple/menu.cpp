#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <algorithm>
#include <array>
#include <iomanip>
#include <mutex>
//#include <numbers>
#include <numeric>
#include <sstream>
#include <vector>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "config.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"

#include "helpers/Helpers.h"
#include "helpers/Memory.h"





struct MiscConfig {
    MiscConfig() { clanTag[0] = '\0'; }
    bool animatedClanTag{ false };
    bool customClanTag{ false };

    char clanTag[16];
} miscConfig;

void Misc::updateClanTag(bool tagChanged) noexcept
{
    static std::string clanTag;

    if (tagChanged) {
        clanTag = miscConfig.clanTag;
        if (!clanTag.empty() && clanTag.front() != ' ' && clanTag.back() != ' ')
            clanTag.push_back(' ');
        return;
    }

    /*static auto lastTime = 0.0f;
    if (miscConfig.customClanTag)
    {
        if (memory->globalVars->realtime - lastTime < 0.6f)
            return;

        if (miscConfig.animatedClanTag && !clanTag.empty()) {
            if (const auto offset = Helpers::utf8SeqLen(clanTag[0]); offset <= clanTag.length())
                std::rotate(clanTag.begin(), clanTag.begin() + offset, clanTag.end());
        }
        lastTime = memory->globalVars->realtime;
        memory->setClanTag(clanTag.c_str(), clanTag.c_str());
    }*/
}


// =========================================================
// 
// These are the tabs on the sidebar
// 
// =========================================================
static char* sidebar_tabs[] = {
    "Home",
    "Visual",
    "Aimbot",
    "Misc",
    "Lua",
    "Chat"
};

constexpr static float get_sidebar_item_width() { return 130.0f; }
constexpr static float get_sidebar_item_height() { return  70.0f; }

enum {
	TAB_HOME,
	TAB_VISUAL,
	TAB_AIMBOT,
	TAB_MISC,
    TAB_LUA,
    TAB_CHAT
};

namespace ImGuiEx           //color picker i think?
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline) //Renders the sidebar tabs
{
    bool values[N] = { false };

    values[activetab] = true;

    for(auto i = 0; i < N; ++i) {
        if(ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
            activetab = i;
        }
        if(sameline && i < N - 1)
            ImGui::SameLine();
    }
}

ImVec2 get_sidebar_size()
{                                               //Size of the sidebar tabs
    constexpr float padding = 10.0f;
    //constexpr auto size_w = padding * 2.0f + get_sidebar_item_width();
    constexpr auto size_w = padding * 2.0f + 800;
    //constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / sizeof(char*)) * get_sidebar_item_height();
    constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / 12) * get_sidebar_item_height();

    //return ImVec2{ size_w, ImMax(325.0f, size_h) };
    return ImVec2{ size_w, (get_sidebar_item_height()+padding*2.0f) };
}

int get_fps()                   //count fps
{
    using namespace std::chrono;
    static int count = 0;
    static auto last = high_resolution_clock::now();
    auto now = high_resolution_clock::now();
    static int fps = 0;

    count++;

    if(duration_cast<milliseconds>(now - last).count() > 1000) {
        fps = count;
        count = 0;
        last = now;
    }

    return fps;
}

void RenderVisualTab()                 //Render Visual tab
{
    static char* esp_tab_names[] = { "ESP", "GLOW", "CHAMS", "WORLD"};
    static int   active_esp_tab = 0;

    bool placeholder_true = true;

    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    {
        render_tabs(esp_tab_names, active_esp_tab, group_w / _countof(esp_tab_names), 25.0f, true);
    }
    ImGui::PopStyleVar();
    ImGui::BeginGroupBox("##body_content");
    {
        if(active_esp_tab == 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", g_Options.esp_enabled);
            ImGui::Checkbox("Team check", g_Options.esp_enemies_only);
            ImGui::Checkbox("Boxes", g_Options.esp_player_boxes);
            ImGui::Checkbox("Names", g_Options.esp_player_names);
            ImGui::Checkbox("Health", g_Options.esp_player_health);
            ImGui::Checkbox("Armour", g_Options.esp_player_armour);
            ImGui::Checkbox("Weapon", g_Options.esp_player_weapons);
            ImGui::Checkbox("Snaplines", g_Options.esp_player_snaplines);

            ImGui::NextColumn();

            ImGui::Checkbox("Crosshair", g_Options.esp_crosshair);
            ImGui::Checkbox("Dropped Weapons", g_Options.esp_dropped_weapons);
            ImGui::Checkbox("Defuse Kit", g_Options.esp_defuse_kit);
            ImGui::Checkbox("Planted C4", g_Options.esp_planted_c4);
			ImGui::Checkbox("Item Esp", g_Options.esp_items);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
            ImGuiEx::ColorEdit3("Allies Visible", g_Options.color_esp_ally_visible);
            ImGuiEx::ColorEdit3("Enemies Visible", g_Options.color_esp_enemy_visible);
            ImGuiEx::ColorEdit3("Allies Occluded", g_Options.color_esp_ally_occluded);
            ImGuiEx::ColorEdit3("Enemies Occluded", g_Options.color_esp_enemy_occluded);
            ImGuiEx::ColorEdit3("Crosshair", g_Options.color_esp_crosshair);
            ImGuiEx::ColorEdit3("Dropped Weapons", g_Options.color_esp_weapons);
            ImGuiEx::ColorEdit3("Defuse Kit", g_Options.color_esp_defuse);
            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_esp_c4);
			ImGuiEx::ColorEdit3("Item Esp", g_Options.color_esp_item);
            ImGui::PopItemWidth();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        } else if(active_esp_tab == 1) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", g_Options.glow_enabled);
            ImGui::Checkbox("Team check", g_Options.glow_enemies_only);
            ImGui::Checkbox("Players", g_Options.glow_players);
            ImGui::Checkbox("Chickens", g_Options.glow_chickens);
            ImGui::Checkbox("C4 Carrier", g_Options.glow_c4_carrier);
            ImGui::Checkbox("Planted C4", g_Options.glow_planted_c4);
            ImGui::Checkbox("Defuse Kits", g_Options.glow_defuse_kits);
            ImGui::Checkbox("Weapons", g_Options.glow_weapons);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
            ImGuiEx::ColorEdit3("Ally", g_Options.color_glow_ally);
            ImGuiEx::ColorEdit3("Enemy", g_Options.color_glow_enemy);
            ImGuiEx::ColorEdit3("Chickens", g_Options.color_glow_chickens);
            ImGuiEx::ColorEdit3("C4 Carrier", g_Options.color_glow_c4_carrier);
            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_glow_planted_c4);
            ImGuiEx::ColorEdit3("Defuse Kits", g_Options.color_glow_defuse);
            ImGuiEx::ColorEdit3("Weapons", g_Options.color_glow_weapons);
            ImGui::PopItemWidth();

            ImGui::NextColumn();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        } else if(active_esp_tab == 2) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::BeginGroupBox("Players");
            {
                ImGui::Checkbox("Enabled", g_Options.chams_player_enabled); ImGui::SameLine();
                ImGui::Checkbox("Team Check", g_Options.chams_player_enemies_only);
                ImGui::Checkbox("Wireframe", g_Options.chams_player_wireframe);
                ImGui::Checkbox("Flat", g_Options.chams_player_flat);
                ImGui::Checkbox("Ignore-Z", g_Options.chams_player_ignorez); ImGui::SameLine();
                ImGui::Checkbox("Glass", g_Options.chams_player_glass);
                ImGui::PushItemWidth(110);
                ImGuiEx::ColorEdit4("Ally (Visible)", g_Options.color_chams_player_ally_visible);
                ImGuiEx::ColorEdit4("Ally (Occluded)", g_Options.color_chams_player_ally_occluded);
                ImGuiEx::ColorEdit4("Enemy (Visible)", g_Options.color_chams_player_enemy_visible);
                ImGuiEx::ColorEdit4("Enemy (Occluded)", g_Options.color_chams_player_enemy_occluded);
                ImGui::PopItemWidth();
            }
            ImGui::EndGroupBox();

            ImGui::NextColumn();

            ImGui::BeginGroupBox("Arms");
            {
                ImGui::Checkbox("Enabled", g_Options.chams_arms_enabled);
                ImGui::Checkbox("Wireframe", g_Options.chams_arms_wireframe);
                ImGui::Checkbox("Flat", g_Options.chams_arms_flat);
                ImGui::Checkbox("Ignore-Z", g_Options.chams_arms_ignorez);
                ImGui::Checkbox("Glass", g_Options.chams_arms_glass);
                ImGui::PushItemWidth(110);
                ImGuiEx::ColorEdit4("Color (Visible)", g_Options.color_chams_arms_visible);
                ImGuiEx::ColorEdit4("Color (Occluded)", g_Options.color_chams_arms_occluded);
                ImGui::PopItemWidth();
            }
            ImGui::EndGroupBox();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        }
        else if (active_esp_tab == 3) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
        ImGui::Columns(3, nullptr, false);
        ImGui::SetColumnOffset(1, group_w / 3.0f);
        ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
        ImGui::SetColumnOffset(3, group_w);

        ImGui::BeginGroupBox("World Modulation");
        {
        ImGui::Checkbox("Third Person", g_Options.misc_thirdperson);
        if (g_Options.misc_thirdperson)
            ImGui::SliderFloat("Distance", g_Options.misc_thirdperson_dist, 0.f, 150.f);
        ImGui::SliderInt("Custom FOV", g_Options.custom_fov, 50, 140);
        ImGui::Text("Postprocessing:");
        ImGui::SliderFloat("Red", g_Options.mat_ambient_light_r, 0, 1);
        ImGui::SliderFloat("Green", g_Options.mat_ambient_light_g, 0, 1);
        ImGui::SliderFloat("Blue", g_Options.mat_ambient_light_b, 0, 1);
        ImGui::Checkbox("Nightmode", g_Options.misc_nightmode);
        if (g_Options.misc_nightmode)
        {
            ImGui::SliderFloat("Nightmode Amount", g_Options.misc_nightmode_amount, 0.65f, 1.f);
            ImGuiEx::ColorEdit4("Nightmode Color", (g_Options.color_nightmode));
        }
        ImGui::EndGroupBox();

        ImGui::Columns(1, nullptr, false);
        ImGui::PopStyleVar();

        }
        }
        
    }
    ImGui::EndGroupBox();
}

void RenderMiscTab()                    //Misc tab
{
    bool placeholder_true = true;

    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::ToggleButton("MISC", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
        ImGui::Columns(3, nullptr, false);
        ImGui::SetColumnOffset(1, group_w / 3.0f);
        ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
        ImGui::SetColumnOffset(3, group_w);

        ImGui::Checkbox("Bunny hop", g_Options.misc_bhop);
        ImGui::Checkbox("No hands", g_Options.misc_no_hands);
		ImGui::Checkbox("Rank reveal", g_Options.misc_showranks);
		//ImGui::PushItemWidth(-1.0f);
		ImGui::NextColumn();
        ImGui::SliderInt("Viewmodel FOV", g_Options.viewmodel_fov, 68, 120);
        //ImGui::PopItemWidth();
        if (ImGui::InputText("", miscConfig.clanTag, sizeof(miscConfig.clanTag)))
            Misc::updateClanTag(true);
        ImGui::Checkbox("Animated clantag", &miscConfig.animatedClanTag);
        ImGui::Columns(1, nullptr, false);
        ImGui::PopStyleVar();
    }
    ImGui::EndGroupBox();
}

void RenderAimbotTab()               //The empty Aimbot tab
{
	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

	bool placeholder_true = true;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::ToggleButton("AIMBOT", &placeholder_true, ImVec2{ group_w, 25.0f });
	ImGui::PopStyleVar();

	ImGui::BeginGroupBox("##body_content");
	{
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
        ImGui::Checkbox("Enable Backtrack", g_Options.aim_backtrack_enabled);
        if (g_Options.aim_backtrack_enabled)
        {
            ImGui::SliderInt("Backtrack Amount", g_Options.aim_backtrack_amount, 0, 200);
        }
	}
	ImGui::EndGroupBox();
}

void RenderLuaTab()               //The empty Lua tab
{
    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    bool placeholder_true = true;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::ToggleButton("LUA", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
        auto message = "There's nothing here. Add something you want!";

        auto pos = ImGui::GetCurrentWindow()->Pos;
        auto wsize = ImGui::GetCurrentWindow()->Size;

        pos = pos + wsize / 2.0f;

        ImGui::RenderText(pos - ImGui::CalcTextSize(message) / 2.0f, message);
    }
    ImGui::EndGroupBox();
}

void RenderChatTab()               //The empty Chat tab
{
    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    bool placeholder_true = true;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::ToggleButton("CHAT", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
        auto message = "There's nothing here. Add something you want!";

        auto pos = ImGui::GetCurrentWindow()->Pos;
        auto wsize = ImGui::GetCurrentWindow()->Size;

        pos = pos + wsize / 2.0f;

        ImGui::RenderText(pos - ImGui::CalcTextSize(message) / 2.0f, message);
    }
    ImGui::EndGroupBox();
}

void RenderHomeTab()                  //Home
{
    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    bool placeholder_true = true;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    ImGui::ToggleButton("CONFIG", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
        ImGui::Checkbox("Watermark##hc", g_Options.misc_watermark);
		if (ImGui::Button("Save cfg")) {
			Config::Get().Save();
		}
		if (ImGui::Button("Load cfg")) {
			Config::Get().Load();
		}
        //ImGuiEx::ColorEdit4("Menu Background color", g_Options.color_menu_background);
    }
    ImGui::EndGroupBox();
}

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;

    if(!_visible)
        return;

    const auto sidebar_size = get_sidebar_size(); //sidebar size
    static int active_sidebar_tab = 0;

    //ImGui::PushStyle(_style);

    ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiSetCond_Once);      //main window position
    ImGui::SetNextWindowSize(ImVec2{ 810, 600 }, ImGuiSetCond_Once); //main window size
	// https://github.com/spirthack/CSGOSimple/issues/63
	// quick fix

	if (ImGui::Begin("CSGOSimple",
		&_visible,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar)) {

		//auto& style = ImGui::GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        {
            ImGui::BeginGroupBox("##sidebar", sidebar_size);
            {
				//ImGui::GetCurrentWindow()->Flags &= ~ImGuiWindowFlags_ShowBorders;
                                    //passes the values through to Render the sidebar tabs
                render_tabs(sidebar_tabs, active_sidebar_tab, get_sidebar_item_width(), get_sidebar_item_height(), true);
            }
            ImGui::EndGroupBox();
        }
        ImGui::PopStyleVar();
        //ImGui::SameLine();

        // Make the body the same vertical size as the sidebar
        // except for the width, which we will set to auto
        auto size = ImVec2{ sidebar_size.x, -30.0f }; //body size

		ImGui::BeginGroupBox("##body", size);
        if(active_sidebar_tab == TAB_HOME) {
            RenderHomeTab();
        } else if(active_sidebar_tab == TAB_VISUAL) {
            RenderVisualTab();
        } else if(active_sidebar_tab == TAB_AIMBOT) {
            RenderAimbotTab();
        } else if(active_sidebar_tab == TAB_MISC) {
            RenderMiscTab();
        } else if(active_sidebar_tab == TAB_LUA) {
            RenderLuaTab();
        } else if (active_sidebar_tab == TAB_CHAT) {
            RenderChatTab();
        }
        
        ImGui::EndGroupBox();
                                                                                          //fps counter
        ImGui::TextColored(ImVec4{ 0.0f, 0.5f, 0.0f, 1.0f }, "FPS: %03d", get_fps());
        //ImGui::TextColored(ImVec4{ 0.0f, 0.5f, 0.0f, 1.0f }, g_Options.color_menu_background, get_fps());
        
                                                                                  //Unload button
        ImGui::SameLine(ImGui::GetWindowWidth() - 305 - ImGui::GetStyle().WindowPadding.x);
        if (ImGui::Button("Configs", ImVec2{ 150, 25 })) {
            g_cfg_open = true;
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 150 - ImGui::GetStyle().WindowPadding.x);
        if(ImGui::Button("Unload", ImVec2{ 150, 25 })) {
            g_Unload = true;
        }
        ImGui::End();
    }
}

void Menu::Toggle()
{
    _visible = !_visible;
}


void Menu::CreateStyle()
{
	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.ChildRounding = 0.f;
	_style.Colors[ImGuiCol_Button]             = ImVec4(0.100f, 0.100f, 0.100f, 0.670f); //_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header]             = ImVec4(0.160f, 0.160f, 0.160f, 0.160f);
	_style.Colors[ImGuiCol_HeaderHovered]      = ImVec4(0.100f, 0.1f, 0.1f, 1.000f);
    _style.Colors[ImGuiCol_Tab]                = ImVec4(0.f, 0.f, 0.f, 0.940f);    //unused i think :face_with_raised_eyebrow: 
    _style.Colors[ImGuiCol_ButtonHovered]      = ImVec4(0.25f, 0.25f, 0.25f, 0.940f);    //hovered top bar/button
	_style.Colors[ImGuiCol_ButtonActive]       = ImVec4(0.35f, 0.35f, 0.35f, 0.940f);    //active top bar tab/button
	_style.Colors[ImGuiCol_FrameBg]            = ImVec4(0.2f, 0.2f, 0.2f, 0.940f);    //Color picker/checkboxes/etc 
	_style.Colors[ImGuiCol_WindowBg]           = ImVec4(0.f, 0.f, 0.f, 0.940f);    //Menu background
	_style.Colors[ImGuiCol_PopupBg]            = ImVec4(0.1f, 0.1f, 0.1f, 0.940f);
    _style.Colors[ImGuiCol_TitleBgActive]      = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    ImGui::GetStyle() = _style;
}
