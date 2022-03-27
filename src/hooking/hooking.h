#pragma once

#include <memory>
#include <type_traits>

#ifdef _WIN32
#include <d3d9.h>
#include <Windows.h>
#elif __linux__
struct SDL_Window;
union SDL_Event;
#endif

#include "minhook.h"
#include "vmtswap.h"

#include "../../lib/sdk/Platform.h"

class matrix3x4;
struct ModelRenderInfo;
struct SoundInfo;

#ifdef _WIN32
using HookType = MinHook;
#else
using HookType = VmtSwap;
#endif

bool deviceInit(IDirect3DDevice9* device);
static bool LoadTextureFromFile(const char* pSrcFile, LPDIRECT3DTEXTURE9* ppTexture, int* width, int* height);

class Hooks {
public:
#ifdef _WIN32
    explicit Hooks(HMODULE moduleHandle) noexcept;

    // --------------------------------------
    // FIXME: Some names might conflict with the global namespace.
    //        Consider changing these variable names to more unique ones.
    LPDIRECT3DTEXTURE9 texture { nullptr };
    int width { 0 };
    int height { 0 };

    std::string fullModulePath;
    // --------------------------------------

    WNDPROC originalWndProc;
    std::add_pointer_t<HRESULT __stdcall(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*)> originalPresent {};
    std::add_pointer_t<HRESULT __stdcall(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)> originalReset {};
#else
    Hooks() noexcept;

    std::add_pointer_t<int(SDL_Event*)> pollEvent;
    std::add_pointer_t<void(SDL_Window*)> swapWindow;
#endif

    void install() noexcept;
    void uninstall() noexcept;
    void callOriginalDrawModelExecute(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept;

    std::add_pointer_t<int FASTCALL_CONV(SoundInfo&)> originalDispatchSound {};

    HookType bspQuery;
    HookType client;
    HookType clientMode;
    HookType engine;
    HookType inventory;
    HookType inventoryManager;
    HookType modelRender;
    HookType panoramaMarshallHelper;
    HookType sound;
    HookType surface;
    HookType viewRender;
    HookType svCheats;

#ifdef _WIN32
    HookType keyValuesSystem;
#endif
private:
#ifdef _WIN32
    HMODULE moduleHandle;
    HWND window;
#endif
};

static LPDIRECT3DDEVICE9 g_pd3dDevice { nullptr };
inline std::unique_ptr<Hooks> hooks;
