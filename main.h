#pragma once
#include <string>
#include <windows.h>

#ifndef MAIN_H
#define MAIN_H

class Main {
public:
    void createConsole();
    bool envshit();
    bool jvmtishit();
    int HookJniFunctions();
    void exit(HMODULE hModule);
    void main(HMODULE hModule);
};

#endif //MAIN_H