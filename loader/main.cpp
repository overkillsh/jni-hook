#define _CRT_SECURE_NO_DEPRECATE
#include "main.h"

#include <Windows.h>
#include <jni.h>
#include <jvmti.h>
#include <cstdio>
#include <algorithm>
#include <string>
#include <iostream>
#include <filesystem>
#include <iostream>
#include <direct.h>
#include <detours.h>

char logFilePath[MAX_PATH];
void Log(const char* format, ...) {
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


JavaVM* jvm;
JNIEnv* jenv;
jvmtiEnv* jvmti;

jclass nativeBridgeClass = nullptr;
jmethodID nativeBridgeTransformClassMethod = nullptr;

void JNICALL Hook_ClassFileLoad(
    jvmtiEnv* jvmti_env,
    JNIEnv* jni_env,
    jclass class_being_redefined,
    jobject loader,
    const char* name,
    jobject protection_domain,
    jint class_data_len,
    const unsigned char* class_data,
    jint* new_class_data_len,
    unsigned char** new_class_data
) {
    if (name == nullptr || class_data == nullptr || class_data_len == 0)
        return;

    LOG("ClassFileLoad: %s", name);

    if (class_being_redefined == nullptr)
        return;
    
    jstring nameJstring = jni_env->NewStringUTF(name);

    // convert class_data to java byte array
    jbyteArray dataByteArray = jni_env->NewByteArray(class_data_len);
    jni_env->SetByteArrayRegion(dataByteArray, 0, class_data_len, reinterpret_cast<const jbyte*>(class_data));

    LOG("calling transform method in classfileloadhook");
    jbyteArray transformedData = dataByteArray;//(jbyteArray) jni_env->CallStaticObjectMethod(nativeBridgeClass, nativeBridgeTransformClassMethod, nameJstring, dataByteArray);
    jni_env->DeleteLocalRef(nameJstring);
    jni_env->DeleteLocalRef(dataByteArray);

    if (!transformedData)
        return;

    jsize transformedLen = jni_env->GetArrayLength(transformedData);
    jbyte* transformedBytes = jni_env->GetByteArrayElements(transformedData, nullptr);

    if (transformedBytes && transformedLen > 0) {
        unsigned char* newBytes = (unsigned char*)malloc(transformedLen);
        if (newBytes) {
            memcpy(newBytes, transformedBytes, transformedLen);
            *new_class_data = newBytes;
            *new_class_data_len = transformedLen;
        }
    }

    jni_env->ReleaseByteArrayElements(transformedData, transformedBytes, JNI_ABORT);
    jni_env->DeleteLocalRef(transformedData);
}

void JNICALL Hook_ClassLoad(
    jvmtiEnv* jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jclass klass) {
    LOG("ClassLoad called: %s", klass);
}

void JNICALL Hook_ClassPrepare(
    jvmtiEnv* jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jclass klass) {
    LOG("ClassPrepare: %s", klass);
}

void JNICALL Hook_FieldModification(
    jvmtiEnv* jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jmethodID method,
    jlocation location,
    jclass field_klass,
    jobject object,
    jfieldID field,
    char signature_type,
    jvalue new_value) {
    LOG("FieldModification: (%s) %s", field_klass, location);
}

void JNICALL Hook_MethodEntry(
    jvmtiEnv* jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jmethodID method) {
    LOG("MethodEntry: %s", method);
}

typedef jint(JNICALL* RegisterNatives_t)(JNIEnv*, jclass, const JNINativeMethod*, jint);
typedef jclass(JNICALL* DefineClass_t) (JNIEnv* env, const char* name, jobject loader, const jbyte* buf, jsize len);
typedef void (JNICALL* SetByteArrayRegion_t)(JNIEnv* env, jbyteArray array, jsize start, jsize len, const jbyte* buf);
typedef void (JNICALL* SetObjectField_t)(JNIEnv*, jobject, jfieldID, jobject);
typedef void (JNICALL* SetLongField_t)(JNIEnv*, jobject, jfieldID, jlong);

typedef jstring (JNICALL* NewString_t)(JNIEnv* env, const jchar* unicode, jsize len);
typedef jstring (JNICALL* NewStringUTF_t)(JNIEnv* env, const char* utf);
typedef jstring (JNICALL* NewStringUTF_t)(JNIEnv* env, const char* utf);
typedef jobject (JNICALL* NewGlobalRef_t)(JNIEnv* env, jobject lobj);

RegisterNatives_t orig_RegisterNatives = nullptr;
DefineClass_t orig_DefineClass = nullptr;
SetByteArrayRegion_t orig_SetByteArrayRegion = nullptr;
SetObjectField_t orig_SetObjectField = nullptr;
SetLongField_t orig_SetLongField = nullptr;
NewString_t orig_NewString = nullptr;
NewStringUTF_t orig_NewStringUTF = nullptr;
NewGlobalRef_t orig_NewGlobalRef = nullptr;

void JNICALL Hook_SetObjectField(JNIEnv* env, jobject obj, jfieldID fieldID, jobject val) {
    LOG("SetObjectField called: %s | %l", fieldID, val);
    return orig_SetObjectField(env, obj, fieldID, val);
}

void JNICALL Hook_SetLongField(JNIEnv* env, jobject obj, jfieldID fieldID, jlong val) {
    LOG("SetLongField called: %s | %l", fieldID, val);
    return orig_SetLongField(env, obj, fieldID, val);
}


jint JNICALL Hook_RegisterNatives(JNIEnv* env, jclass clazz, const JNINativeMethod* methods, jint numMethods) {
    LOG("RegisterNatives called: %s | %p (%s)", methods->name, reinterpret_cast<uintptr_t>(methods->fnPtr), clazz);
    //DebugBreak(); // :3
    return orig_RegisterNatives(env, clazz, methods, numMethods);
}

jclass JNICALL Hook_DefineClass(JNIEnv* env, const char* name, jobject loader, const jbyte* buf, jsize len) {
    LOG("DefineClass called for class: %s | %s", name, loader);
    return orig_DefineClass(env, name, loader, buf, len);
}

jstring JNICALL Hook_NewString(JNIEnv* env, const jchar* unicode, jsize len) {
    LOG("NewString called: %s", unicode);
    return orig_NewString(env, unicode, len);
}

jstring JNICALL Hook_NewStringUTF(JNIEnv* env, const char* utf) {
    LOG("NewStringUTF called: %s", utf);
    return orig_NewStringUTF(env, utf);
}

void JNICALL Hook_SetByteArrayRegion(JNIEnv* env, jbyteArray array, jsize start, jsize len, const jbyte* buf) {
    //LOG("SetByteArrayRegion called: %d", len);
    return orig_SetByteArrayRegion(env, array, start, len, buf);
}

jobject JNICALL Hook_NewGlobalRef(JNIEnv* env, jobject lobj) {
    LOG("NewGlobalRef called: %s", lobj);
    return orig_NewGlobalRef(env, lobj);
}

void Main::createConsole() {
    AllocConsole();
    SetConsoleTitle("unicrack client rale?");

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
    return true;
}

void HookJniFunctions() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    orig_RegisterNatives = jenv->functions->RegisterNatives;
    orig_DefineClass = jenv->functions->DefineClass;
    orig_SetByteArrayRegion = jenv->functions->SetByteArrayRegion;

    orig_SetObjectField = jenv->functions->SetObjectField;
    orig_SetLongField = jenv->functions->SetLongField;

    orig_NewString = jenv->functions->NewString;
    orig_NewStringUTF = jenv->functions->NewStringUTF;

    orig_NewGlobalRef = jenv->functions->NewGlobalRef;

    DetourAttach(&(PVOID&)orig_RegisterNatives, Hook_RegisterNatives);
    DetourAttach(&(PVOID&)orig_DefineClass, Hook_DefineClass);

    DetourAttach(&(PVOID&)orig_SetByteArrayRegion, Hook_SetByteArrayRegion);
    //DetourAttach(&(PVOID&)orig_SetObjectField, Hook_SetObjectField);
    //DetourAttach(&(PVOID&)orig_SetLongField, Hook_SetLongField);

    DetourAttach(&(PVOID&)orig_NewString, Hook_NewString);
    DetourAttach(&(PVOID&)orig_NewStringUTF, Hook_NewStringUTF);

    DetourAttach(&(PVOID&)orig_NewGlobalRef, Hook_NewGlobalRef);

    DetourTransactionCommit();

    LOG("hooked RegisterNatives %p | orig %p", orig_RegisterNatives, Hook_RegisterNatives);
    LOG("hooked DefineClass %p | orig %p", orig_DefineClass, Hook_DefineClass);
    LOG("hooked SetByteArrayRegion %p | orig %p", orig_SetByteArrayRegion, Hook_SetByteArrayRegion);
    //LOG("hooked SetObjectField %p | orig %p", orig_SetObjectField, SetObjectFieldHook);
    //LOG("hooked SetLongField %p | orig %p", orig_SetLongField, SetLongFieldHook);
    LOG("hooked NewGlobalRef %p | orig %p", orig_NewGlobalRef, Hook_NewGlobalRef);
}

//extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {

//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
//    jvm = vm;
//    if (jvm->GetEnv(reinterpret_cast<void**>(&jenv), JNI_VERSION_1_6) != JNI_OK) {
//        LOG("invalid JNI_OnLoad ver");
//        return JNI_ERR;
//    }
//    LOG("JNI_OnLoad called");
//
//    HookJniFunctions();
//
//    return JNI_VERSION_1_6;
//}

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

    jvmtiEventCallbacks callbacks = { 0 };
    callbacks.ClassFileLoadHook = Hook_ClassFileLoad;
    //callbacks.ClassLoad = Hook_ClassLoad;
    callbacks.ClassPrepare = Hook_ClassPrepare;
    //callbacks.FieldModification = Hook_FieldModification;
    //callbacks.MethodEntry = Hook_MethodEntry;

    if (jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks)) != JVMTI_ERROR_NONE) {
        LOG("failed to set jvmti callbacks");
        return false;
    }

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


    HookJniFunctions();
    LOG("all init");

    while (!GetAsyncKeyState(VK_F7)) {

    }

    exit(hModule);
}