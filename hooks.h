#pragma once
#include "utils.h"

#ifndef HOOKS_H
#define HOOKS_H

namespace Hooks {
    //class Hooks {
    //private:
    typedef jint(JNICALL* RegisterNatives_t)(JNIEnv*, jclass, const JNINativeMethod*, jint);
    typedef jclass(JNICALL* DefineClass_t) (JNIEnv* env, const char* name, jobject loader, const jbyte* buf, jsize len);
    typedef void (JNICALL* SetByteArrayRegion_t)(JNIEnv* env, jbyteArray array, jsize start, jsize len, const jbyte* buf);
    typedef void (JNICALL* SetObjectField_t)(JNIEnv*, jobject, jfieldID, jobject);
    typedef void (JNICALL* SetLongField_t)(JNIEnv*, jobject, jfieldID, jlong);

    typedef jstring(JNICALL* NewString_t)(JNIEnv* env, const jchar* unicode, jsize len);
    typedef jstring(JNICALL* NewStringUTF_t)(JNIEnv* env, const char* utf);
    typedef jstring(JNICALL* NewStringUTF_t)(JNIEnv* env, const char* utf);
    typedef jobject(JNICALL* NewGlobalRef_t)(JNIEnv* env, jobject lobj);

    //public:
    extern RegisterNatives_t orig_RegisterNatives;
    extern DefineClass_t orig_DefineClass;
    extern SetByteArrayRegion_t orig_SetByteArrayRegion;
    extern SetObjectField_t orig_SetObjectField;
    extern SetLongField_t orig_SetLongField;
    extern NewString_t orig_NewString;
    extern NewStringUTF_t orig_NewStringUTF;
    extern NewGlobalRef_t orig_NewGlobalRef;


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
    );

    void JNICALL Hook_ClassLoad(
        jvmtiEnv* jvmti_env,
        JNIEnv* jni_env,
        jthread thread,
        jclass klass);

    void JNICALL Hook_ClassPrepare(
        jvmtiEnv* jvmti_env,
        JNIEnv* jni_env,
        jthread thread,
        jclass klass);

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
        jvalue new_value);

    void JNICALL Hook_MethodEntry(
        jvmtiEnv* jvmti_env,
        JNIEnv* jni_env,
        jthread thread,
        jmethodID method);
    // ========================================================================================================================== //

    // JNI HOOKS
    void JNICALL Hook_SetObjectField(JNIEnv* env, jobject obj, jfieldID fieldID, jobject val);
    void JNICALL Hook_SetLongField(JNIEnv* env, jobject obj, jfieldID fieldID, jlong val);


    jint JNICALL Hook_RegisterNatives(JNIEnv* env, jclass clazz, const JNINativeMethod* methods, jint numMethods);
    jclass JNICALL Hook_DefineClass(JNIEnv* env, const char* name, jobject loader, const jbyte* buf, jsize len);

    jstring JNICALL Hook_NewString(JNIEnv* env, const jchar* unicode, jsize len);
    jstring JNICALL Hook_NewStringUTF(JNIEnv* env, const char* utf);

    void JNICALL Hook_SetByteArrayRegion(JNIEnv* env, jbyteArray array, jsize start, jsize len, const jbyte* buf);
    jobject JNICALL Hook_NewGlobalRef(JNIEnv* env, jobject lobj);
    // ========================================================================================================================== //

    NTSTATUS Init(JNIEnv* jenv);
    jvmtiError HookJVMTI(jvmtiEnv* jvmti);
}
#endif // HOOKS_H