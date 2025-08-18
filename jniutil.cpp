#include "jniutil.h"

JNIUtil::JNIUtil(JNIEnv* env) : jenv(env) {

}

jstring JNIUtil::stringToJstring(const char* str) {
	return jenv->NewStringUTF(str);
}

const char* JNIUtil::jstringToString(jstring jstr) {
	const char* str = jenv->GetStringUTFChars(jstr, 0);
	jenv->ReleaseStringUTFChars(jstr, str);
	return str;
}

jclass JNIUtil::findClass(const char* className) {
	// TODO: implement searching classloader maybe aswell fr
	return jenv->FindClass(className);
}

jobject JNIUtil::instantiate(const char* className, const char* constructorDesc, ...) {
	jclass jclazz = findClass(className);
	if (jclazz == nullptr)
		return nullptr;

	jmethodID constructor = jenv->GetMethodID(jclazz, "<init>", constructorDesc);
	if (constructor == nullptr)
		return nullptr;

	va_list args;
	va_start(args, constructorDesc);

	jobject jobj = jenv->NewObjectV(jclazz, constructor, args);

	va_end(args);

	return jobj;
}