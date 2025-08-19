#pragma once
#include <Windows.h>
#include <ntstatus.h>
//#include <ntdef.h>
#include <iostream>
#include <detours.h>

#include "jniutil.h"

#define _SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#ifndef UTILS_H
#define UTILS_H

extern char logFilePath[MAX_PATH];
inline void Log(const char* format, ...) {
    va_list args;
    va_start(args, format);

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
#define LOG(...) Log(__VA_ARGS__)

// shitcode/forward declaration
extern JNIUtil* JUtil;
#endif