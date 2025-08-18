#pragma once
#include <jni.h>

#ifndef JNIUTIL_H
#define JNIUTIL_H

class JNIUtil {
public:
    JNIUtil(JNIEnv* jenv);
    jstring stringToJstring(const char* string);
    const char* jstringToString(jstring jstring);
    jclass findClass(const char* className);
    jobject instantiate(const char* className, const char* constructorDesc, ...);

private:
    JNIEnv* jenv;
};

#endif //JNIUTIL_H