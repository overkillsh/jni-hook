#pragma once
#include <Windows.h>
#include <iostream>
#include <detours.h>

#include <jni.h>
#include <jvmti.h>


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

// stackoverflow
inline std::string getClassName(JNIEnv* env, jclass klass, bool fullpath = true)
{
    jclass clazz = env->FindClass("java/lang/Class");
    jmethodID mid_getName = env->GetMethodID(clazz, "getName", "()Ljava/lang/String;");
    jstring strObj = (jstring)env->CallObjectMethod(klass, mid_getName);
    const char* localName = env->GetStringUTFChars(strObj, 0);
    std::string res = localName;
    env->ReleaseStringUTFChars(strObj, localName);
    if (!fullpath)
    {
        std::size_t pos = res.find_last_of('.');
        if (pos != std::string::npos)
        {
            res = res.substr(pos + 1);
        }
    }
    return res;
}