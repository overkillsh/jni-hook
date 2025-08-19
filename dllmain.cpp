#include "main.h"
#include <Windows.h>
#include <thread>

void load(HMODULE hModule) {
    Main main = Main();
    main.main(hModule);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    // no attach, no run :fire:
    if (reason != DLL_PROCESS_ATTACH) return TRUE;

    std::thread{ load, hModule }.detach();
    
    return TRUE;
}
