#pragma once
#include <jni.h>
#include <jvmti.h>
#include <string>

#ifndef JNIUTIL_H
#define JNIUTIL_H

class JNIUtil {
public:
    //JNIUtil();
    JNIUtil(JNIEnv* jenv);
    //static JNIUtil* getInstance() { return &instance; }
    jstring StringToJstring(const char* string);
    const char* JstringToString(jstring jstring);
    jclass FindClass(const char* className);
    jobject Instantiate(const char* className, const char* constructorDesc, ...);
    // GetClassName winapi exists are we fr
    std::string GetJClassName(JNIEnv* env, jclass klass, bool fullpath = true);
private:
    //static JNIUtil instance;
    JNIEnv* jenv;
};

#endif //JNIUTIL_H