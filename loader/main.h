#pragma once
#include "jni.h"
#include <Windows.h>
#include <string>

#ifndef MAIN_H
#define MAIN_H

class Main {
public:
    void createConsole();
    bool envshit();
    bool jvmtishit();
    void exit(HMODULE hModule);
    void main(HMODULE hModule);
    jobject loadJar(std::string path);
};

#endif //MAIN_H