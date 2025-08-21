#include "jniutil.h"

// make util to auto enum a jobject for more info

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

std::string JNIUtil::GetName(JNIEnv* env, jclass obj, bool fullpath)
{
	if (!jenv) return "erm";
	jclass clazz = env->FindClass("java/lang/Class");
	jmethodID mID = env->GetMethodID(obj, "getName", "()Ljava/lang/String;");
	jstring strObj = (jstring)env->CallObjectMethod(obj, mID);
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

std::string JNIUtil::jcharToString(const jchar* unicode, jsize len) {
	std::wstring wstr(unicode, unicode + len);
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8Str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], size_needed, nullptr, nullptr);
	return utf8Str;


	/*std::wstring wstr(unicode, unicode + len);
	std::string utf8Str;

	for (wchar_t wc : wstr) {
		if (wc < 0x80) {
			utf8Str += static_cast<char>(wc);
		}
		else if (wc < 0x800) {
			utf8Str += static_cast<char>((wc >> 6) | 0xC0);
			utf8Str += static_cast<char>((wc & 0x3F) | 0x80);
		}
		else {
			utf8Str += static_cast<char>((wc >> 12) | 0xE0);
			utf8Str += static_cast<char>(((wc >> 6) & 0x3F) | 0x80);
			utf8Str += static_cast<char>((wc & 0x3F) | 0x80);
		}
	}

	return utf8Str;*/
}

//JNIUtil JNIUtil::instance;