#include <memory>
#include <iostream>

#ifdef _WIN32
#include <clocale>
#include <Windows.h>
#endif

#include "hooking/hooking.h"

#ifdef _WIN32

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved) {
    if (!_CRT_INIT(moduleHandle, reason, reserved))
        return FALSE;

    if (reason == DLL_PROCESS_ATTACH) {
#ifdef _DEBUG
        AllocConsole();
        SetConsoleTitleA("[slippery.gg] - Developer Console");
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
#endif

        std::setlocale(LC_CTYPE, ".utf8");
        hooks = std::make_unique<Hooks>(moduleHandle);

        LPSTR tempModulePath {};
        if (!GetModuleFileNameA(moduleHandle, tempModulePath, MAX_PATH))
            assert("Failed to get module file name!");
        std::string temp { tempModulePath };
        auto moduleSize { sizeof("slippery.dll") + 1 };
        temp.replace((temp.size() - moduleSize), moduleSize, "res\\background.png");
        hooks->fullModulePath = temp;
    }
    return TRUE;
}

#else

void __attribute__((constructor)) DllEntryPoint() {
    hooks = std::make_unique<Hooks>();
}

#endif
