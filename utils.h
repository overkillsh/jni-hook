#pragma once
#include <Windows.h>
#include <ntstatus.h>
//#include <ntdef.h>
#include <iostream>
#include <detours.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <psapi.h>

#include "jniutil.h"

#define _SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#ifndef UTILS_H
#define UTILS_H

extern char logFilePath[MAX_PATH];
extern char tmpPath[MAX_PATH];
inline void Log(bool includeTime, const char* format, ...) {
    va_list args;
    va_start(args, format);

    if (includeTime) printf("[%s] ", std::to_string(time(0)).c_str());
    vprintf(format, args);
    printf("\n");

    FILE* logFile;
    fopen_s(&logFile, logFilePath, "a");
    if (logFile) {
        va_start(args, format);
        vfprintf(logFile, format, args);
        fprintf(logFile, "\n");

        fclose(logFile);
    }
    va_end(args);
}
// make betterer soon:tm:
#define LOG(...) Log(true, __VA_ARGS__)
#define LOGA(...) Log(false, __VA_ARGS__)


inline int SaveToDisk(const char* filename, unsigned char* buf, int len) {
    FILE* pFile;
    int result = fopen_s(&pFile, filename, "wb");
    if (result != 0 || pFile == nullptr) {
        LOG("failed to open file: %s", filename);
        return 1;
    }

    size_t writtenLen = fwrite(buf, sizeof(unsigned char), len, pFile);
    if (writtenLen != len) {
        printf("failed to write file: %s | %d", filename, len);
        fclose(pFile);
        return 2;
    }

    fclose(pFile);
    return 0;
}

inline std::string getFilePath(const std::string& basePath, const std::string& baseName) {
    std::string fullPath = basePath + "dump\\" + baseName;
    int counter = 1;

    while (std::filesystem::exists(fullPath + ".class")) {
        fullPath = basePath + "dump\\" + baseName + std::to_string(counter);
        counter++;
    }

    return fullPath;
}


inline bool isAscii(const std::string& str) {
    for (char c : str) {
        if (static_cast<unsigned char>(c) > 127) {
            return false;
        }
    }
    return true;
}

inline std::string toHex(const std::string& str) {
    std::ostringstream oss;
    for (unsigned char c : str) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return oss.str();
}

inline DWORD CalcRVA(uintptr_t ptr) {
    HMODULE hModule = NULL;
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(ptr), &hModule)) { // | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
        LOG("(error) failed CalcRVA getmodulehandle: %u", GetLastError());
        return -1;
    }

    //MODULEINFO moduleInfo;
    //if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
    //if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
        //return -1;
    //}
    //LOG("testing");
    DWORD rva = static_cast<DWORD>(ptr - reinterpret_cast<uintptr_t>(hModule));//(DWORD)moduleInfo.lpBaseOfDll;
    return rva;
}


// shitcode/forward declaration
extern JNIUtil* JUtil;
#endif