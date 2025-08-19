#define _CRT_SECURE_NO_DEPRECATE
#include "main.h"
#include "hooks.h"

#include <cstdio>
#include <algorithm>
#include <string>
#include <iostream>
#include <filesystem>
#include <direct.h>


JavaVM* jvm;
JNIEnv* jenv;
jvmtiEnv* jvmti;
JNIUtil* JUtil = nullptr;

jclass nativeBridgeClass = nullptr;
jmethodID nativeBridgeTransformClassMethod = nullptr;


void Main::createConsole() {
    AllocConsole();
    SetConsoleTitle("unicrack client rale? (i miss nikko)");

    FILE* file;
    freopen_s(&file, "CONOUT$", "w", stdout);
}

bool Main::envshit() {
    if (JNI_GetCreatedJavaVMs(&jvm, 1, nullptr) != JNI_OK) {
        LOG("failed to get created java vms");
        return false;
    }

    if (jvm->AttachCurrentThread(reinterpret_cast<void**>(&jenv), nullptr) != JNI_OK) {
        LOG("failed to attach to the current thread");
        return false;
    }


    LOG("found jvm and attached to the current threadfr");

    JUtil = new JNIUtil(jenv);
    LOG("initialised JNIUtil");

    return true;
}

NTSTATUS Main::HookJniFunctions() {
    return Hooks::Init(jenv);
}

bool Main::jvmtishit() {
    if (jvm->GetEnv(reinterpret_cast<void**>(&jvmti), JVMTI_VERSION_1_2) != JNI_OK) {
        LOG("failed to get jvmti");
        return false;
    }

    jvmtiCapabilities capabilities = { 0 };
    // get all capabilities (default struct is filled with 1s
    //jvmtiCapabilities capabilities;
    capabilities.can_generate_all_class_hook_events = 1;
    capabilities.can_retransform_classes = 1;

    // cant use these (probably OnLoad only)
    //capabilities.can_generate_field_modification_events = 1;
    //capabilities.can_generate_method_entry_events = 1;

    if (jvmti->AddCapabilities(&capabilities) != JVMTI_ERROR_NONE) {
        LOG("failed to add jvmti capabilities");
        return false;
    }

    if (!Hooks::HookJVMTI(jvmti)) return false;

    //jvmtiEvent events = JVMTI_EVENT_CLASS_FILE_LOAD_HOOK | JVMTI_EVENT_METHOD_ENTRY;
    //jvmtiEvent[] events = { JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, JVMTI_EVENT_METHOD_ENTRY };

    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr) != JVMTI_ERROR_NONE) {
        LOG("failed to enable jvmti event notifications");
        return false;
    }

    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, nullptr) != JVMTI_ERROR_NONE) {
        LOG("failed to enable jvmti event notifications 1");
        return false;
    }

    /*if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION, nullptr) != JVMTI_ERROR_NONE) {
        LOG("failed to enable jvmti event notifications 1");
        return false;
    }

    if (jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, nullptr) != JVMTI_ERROR_NONE) {
        LOG("failed to enable jvmti event notifications 2");
        return false;
    }*/

    LOG("jvmti success");
    return true;
}

void Main::exit(HMODULE hModule) {
    LOG("exiting");

    if (jvmti) {
        jvmti->SetEventNotificationMode(JVMTI_DISABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    }

    if (jvm) {
        jvm->DetachCurrentThread();
    }

    Sleep(500);

    CreateThread(nullptr, 0, [](LPVOID mod) -> DWORD {
        Sleep(500);
        FreeLibraryAndExitThread((HMODULE)mod, 0);
        return 0;
    }, hModule, 0, nullptr);
}

char logFilePath[MAX_PATH];
void Main::main(HMODULE hModule) {
    DWORD tempPathLength = GetTempPathA(MAX_PATH, logFilePath);
    if (tempPathLength == 0 || tempPathLength > MAX_PATH) {
        return;
    }
    strcat_s(logFilePath, MAX_PATH, "log.txt");
    FILE* logFile;
    fopen_s(&logFile, logFilePath, "w");
    if (logFile) fclose(logFile);

    createConsole();

    if (!envshit() || !jvmtishit()) {
        exit(hModule);
        return;
    }


    if (!STATUS_SUCCESS(HookJniFunctions())) {
        LOG("failed to hook jni func ptrs");
        return;
    }

    LOG("all init success");
    while (!GetAsyncKeyState(VK_F7)) {

    }

    exit(hModule);
}