#include "jniutil.h"


//JNIUtil::JNIUtil() {}
JNIUtil::JNIUtil(JNIEnv* env) : jenv(env) {}

jstring JNIUtil::StringToJstring(const char* str) {
	return jenv->NewStringUTF(str);
}

const char* JNIUtil::JstringToString(jstring jstr) {
	const char* str = jenv->GetStringUTFChars(jstr, 0);
	jenv->ReleaseStringUTFChars(jstr, str);
	return str;
}

jclass JNIUtil::FindClass(const char* className) {
	// TODO: implement searching classloader maybe aswell fr
	return jenv->FindClass(className);
}

jobject JNIUtil::Instantiate(const char* className, const char* constructorDesc, ...) {
	jclass jclazz = FindClass(className);
	if (jclazz == nullptr) return nullptr;

	jmethodID constructor = jenv->GetMethodID(jclazz, "<init>", constructorDesc);
	if (constructor == nullptr) return nullptr;

	va_list args;
	va_start(args, constructorDesc);

	jobject jobj = jenv->NewObjectV(jclazz, constructor, args);

	va_end(args);

	return jobj;
}

// stackoverflow
std::string JNIUtil::GetJClassName(JNIEnv* env, jclass klass, bool fullpath)
{
	//if (!jenv) return "erm";
	jclass clazz = env->FindClass("java/lang/Class");
	jmethodID mID = env->GetMethodID(clazz, "getName", "()Ljava/lang/String;");
	jstring strObj = (jstring)env->CallObjectMethod(klass, mID);
	//std::string res = JstringToString(strObj);
	const char* str = env->GetStringUTFChars(strObj, 0);
	std::string res = str;
	env->ReleaseStringUTFChars(strObj, str);
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

//JNIUtil JNIUtil::instance;