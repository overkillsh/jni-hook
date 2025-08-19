#pragma once
#include "hooks.h"

// should probably make this oop
namespace Hooks {
    RegisterNatives_t orig_RegisterNatives = nullptr;
    DefineClass_t orig_DefineClass = nullptr;
    SetByteArrayRegion_t orig_SetByteArrayRegion = nullptr;
    SetObjectField_t orig_SetObjectField = nullptr;
    SetLongField_t orig_SetLongField = nullptr;
    NewString_t orig_NewString = nullptr;
    NewStringUTF_t orig_NewStringUTF = nullptr;
    NewGlobalRef_t orig_NewGlobalRef = nullptr;
    //class Hooks {
    //private:

    //public:


    // JVMTI HOOKS
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

        //LOG("ClassFileLoad: %s", name);

        if (class_being_redefined == nullptr)
            return;

        jstring nameJstring = jni_env->NewStringUTF(name);

        // convert class_data to java byte array
        jbyteArray dataByteArray = jni_env->NewByteArray(class_data_len);
        jni_env->SetByteArrayRegion(dataByteArray, 0, class_data_len, reinterpret_cast<const jbyte*>(class_data));

        LOG("ClassFileLoad retransform: (%s) len: %l", name, new_class_data_len);
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
        //LOG("ClassLoad called: %s", thread);
        std::string name = JUtil->GetJClassName(jni_env, klass);
        LOG("ClassLoad called: %s", name);
    }

    void JNICALL Hook_ClassPrepare(
        jvmtiEnv* jvmti_env,
        JNIEnv* jni_env,
        jthread thread,
        jclass klass) {
        /*LOG("ClassPrepare: %s", klass);*/
        std::string name = JUtil->GetJClassName(jni_env, klass);
        LOG("ClassPrepare called: %s", name);
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
        //LOG("FieldModification: (%s) %s", field_klass, location);
    }

    void JNICALL Hook_MethodEntry(
        jvmtiEnv* jvmti_env,
        JNIEnv* jni_env,
        jthread thread,
        jmethodID method) {
        //LOG("MethodEntry: %s", method);
    }
    // ========================================================================================================================== //

    // JNI HOOKS
    void JNICALL Hook_SetObjectField(JNIEnv* env, jobject obj, jfieldID fieldID, jobject val) {
        LOG("SetObjectField called: %s | %l", fieldID, val);
        return orig_SetObjectField(env, obj, fieldID, val);
    }

    void JNICALL Hook_SetLongField(JNIEnv* env, jobject obj, jfieldID fieldID, jlong val) {
        LOG("SetLongField called: %s | %l", fieldID, val);
        return orig_SetLongField(env, obj, fieldID, val);
    }


    jint JNICALL Hook_RegisterNatives(JNIEnv* env, jclass clazz, const JNINativeMethod* methods, jint numMethods) {
        std::string clazzName = JUtil->GetJClassName(env, clazz);
        LOG("RegisterNatives called: %s | %p (%s)", methods->name, reinterpret_cast<uintptr_t>(methods->fnPtr), clazzName);
        //DebugBreak(); // :3
        return orig_RegisterNatives(env, clazz, methods, numMethods);
    }

    jclass JNICALL Hook_DefineClass(JNIEnv* env, const char* name, jobject loader, const jbyte* buf, jsize len) {
        LOG("DefineClass called for class: %s | %s", name, loader);
        return orig_DefineClass(env, name, loader, buf, len);
    }

    jstring JNICALL Hook_NewString(JNIEnv* env, const jchar* unicode, jsize len) {
        //LOG("NewString called: %s", unicode);
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
        LOG("NewGlobalRef called: %p", lobj);
        return orig_NewGlobalRef(env, lobj);
    }
    // ========================================================================================================================== //



    NTSTATUS Init(JNIEnv* jenv) {
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

        return STATUS_SUCCESS;
        //return HookJVMTI();
    }

    jvmtiError HookJVMTI(jvmtiEnv* jvmti) {
        jvmtiEventCallbacks callbacks = { 0 };
        callbacks.ClassFileLoadHook = Hook_ClassFileLoad;
        //callbacks.ClassLoad = Hook_ClassLoad;
        callbacks.ClassPrepare = Hook_ClassPrepare;
        //callbacks.FieldModification = Hook_FieldModification;
        //callbacks.MethodEntry = Hook_MethodEntry;

        jvmtiError status = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
        if (status != JVMTI_ERROR_NONE) {
            LOG("failed to set jvmti callbacks");
        }

        return status;
    }
}