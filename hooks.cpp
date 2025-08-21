#pragma once
#include "hooks.h"

// should probably make this oop
namespace Hooks {
    jvmtiEnv* jvmti = nullptr;

    RegisterNatives_t orig_RegisterNatives = nullptr;
    DefineClass_t orig_DefineClass = nullptr;
    SetByteArrayRegion_t orig_SetByteArrayRegion = nullptr;
    SetObjectField_t orig_SetObjectField = nullptr;
    SetLongField_t orig_SetLongField = nullptr;
    NewString_t orig_NewString = nullptr;
    NewStringUTF_t orig_NewStringUTF = nullptr;
    NewGlobalRef_t orig_NewGlobalRef = nullptr;
    CallObjectMethod_t orig_CallObjectMethod = nullptr;
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
        //std::string name = JUtil->GetJClassName(jni_env, klass);
        //LOG("ClassPrepare called: %s", name);
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
        jclass clazz = env->GetObjectClass(obj);
        if (clazz == nullptr) return orig_SetObjectField(env, obj, fieldID, val);

        char* name = nullptr;
        char* sig = nullptr;
        char* generic = nullptr;
        jvmtiError error = jvmti->GetFieldName(clazz, fieldID, &name, &sig, &generic);
        if (error != JVMTI_ERROR_NONE) {
            LOG("(error) failed getting field: %s", clazz);
            return orig_SetObjectField(env, obj, fieldID, val);
        }

        const char* fieldName = name ? name : "unknown";

        if (name) jvmti->Deallocate((unsigned char*)name);
        if (sig) jvmti->Deallocate((unsigned char*)sig);
        if (generic) jvmti->Deallocate((unsigned char*)generic);

        LOG("-| (%s) SetObjectField: %s [%s] (%p) | %l", JUtil->GetJClassName(env, clazz).c_str(), fieldName, sig, fieldID, val);
        return orig_SetObjectField(env, obj, fieldID, val);
    }

    void JNICALL Hook_SetLongField(JNIEnv* env, jobject obj, jfieldID fieldID, jlong val) {
        LOG("SetLongField called: %s | %l", fieldID, val);
        return orig_SetLongField(env, obj, fieldID, val);
    }


    jint JNICALL Hook_RegisterNatives(JNIEnv* env, jclass clazz, const JNINativeMethod* methods, jint numMethods) {
        std::string clazzName = JUtil->GetJClassName(env, clazz);
        LOGA("");
        LOG("-| (%s) registering %d natives", clazzName.c_str(), numMethods);
        for (int i = 0; i < numMethods; i++) {
            const JNINativeMethod* method = &methods[i];
            uintptr_t funcPtr = reinterpret_cast<uintptr_t>(method->fnPtr);
            DWORD rva = CalcRVA(funcPtr);
            LOGA(" -> %s [%s] {} | rva: %u (%p)", method->name, method->signature, rva, funcPtr);
        }
        LOGA("");
        //DebugBreak(); // :3
        return orig_RegisterNatives(env, clazz, methods, numMethods);
    }

    jclass JNICALL Hook_DefineClass(JNIEnv* env, const char *name, jobject loader, const jbyte* buf, jsize len) {
        if (buf == NULL || len <= 0) return orig_DefineClass(env, name, loader, buf, len);
        jclass clazz = env->GetObjectClass(loader);
        if (clazz == nullptr) return orig_DefineClass(env, name, loader, buf, len);

        unsigned char* newBuf = (unsigned char*)malloc(len);
        if (newBuf == NULL) return orig_DefineClass(env, name, loader, buf, len);

        memcpy(newBuf, buf, len);

        std::string tmpName = "clazz";//std::to_string(time(0)).c_str();//ptrString;
        std::string filename;
        
        if (name != nullptr) filename = name;
        else {
            filename = getFilePath(tmpPath, tmpName);
            //LOG("meow %s", filename.c_str());
        }

        std::string filepath = filename + ".class";//(isAscii(finalName)) ? finalName : toHex(finalName);//std::to_string(bruh);

        //if (strstr(className, "java.lang")) return orig_DefineClass(env, name, loader, buf, len);
        LOG("-| (%s) DefineClass: %s | %d", JUtil->GetJClassName(env, clazz).c_str(), filename.c_str(), len);
        
        //char filepath[MAX_PATH];
        //snprintf(filepath, MAX_PATH, "%s%s%s", tmpPath, className.c_str(), ".class");
        SaveToDisk(filepath.c_str(), newBuf, len);
        free(newBuf);
        
        return orig_DefineClass(env, name, loader, buf, len);
    }

    jstring JNICALL Hook_NewString(JNIEnv* env, const jchar* unicode, jsize len) {
        //LOG("NewString called: %s", unicode);
        LOG("NewString: %s", JUtil->jcharToString(unicode, len).c_str());
        return orig_NewString(env, unicode, len);
    }

    jstring JNICALL Hook_NewStringUTF(JNIEnv* env, const char* utf) {
        LOG("NewStringUTF: %s", utf);
        return orig_NewStringUTF(env, utf);
    }

    void JNICALL Hook_SetByteArrayRegion(JNIEnv* env, jbyteArray array, jsize start, jsize len, const jbyte* buf) {
        //LOG("SetByteArrayRegion called: %d", len);
        return orig_SetByteArrayRegion(env, array, start, len, buf);
    }

    jobject JNICALL Hook_NewGlobalRef(JNIEnv* env, jobject lobj) {
        //LOG("NewGlobalRef called: %p", lobj);
        return orig_NewGlobalRef(env, lobj);
    }

    // gonna have to convert method sig into format for printing all args (since we dont know the types) :/ soon:tm:
    jobject JNICALL Hook_CallObjectMethod(JNIEnv* env, jobject obj, jmethodID methodID, ...) {
        char* name = nullptr;
        char* sig = nullptr;
        char* generic = nullptr;
        char* classSig = nullptr;

        const char* methodName = "unknown";
        jvmtiError error;

        //std::vector<void*> jArgs;
        //std::ostringstream oss;

        va_list args;
        va_start(args, methodID);

        //jclass clazz = env->GetObjectClass(obj);
        //if (clazz == nullptr) goto QUICK_EXIT;
        jclass clazz;
        error = jvmti->GetMethodDeclaringClass(methodID, &clazz);
        if (error != JVMTI_ERROR_NONE || clazz == nullptr) {
            LOG("(error) failed getting declaring class for method: %p", methodID);
            goto QUICK_EXIT;
        }

        error = jvmti->GetClassSignature(clazz, &classSig, nullptr);
        if (error != JVMTI_ERROR_NONE) {
            LOG("(error) failed getting class signature for method: %p", methodID);
            goto QUICK_EXIT;
        }

        
        error = jvmti->GetMethodName(methodID, &name, &sig, &generic);
        if (error != JVMTI_ERROR_NONE) {
            LOG("(error) failed getting method: %s", methodID);
            goto QUICK_EXIT;
        }

        if (name) methodName = name;

        if (name) jvmti->Deallocate((unsigned char*)name);
        if (sig) jvmti->Deallocate((unsigned char*)sig);
        if (generic) jvmti->Deallocate((unsigned char*)generic);
        if (classSig) jvmti->Deallocate((unsigned char*)classSig);

        // temp until we convert sig into format
       /* for (int i = 0; i < 10; ++i) {
            void* arg = va_arg(args, void*);
            jArgs.push_back(arg);
        }
        oss << "args: ";
        for (const auto& ptr : jArgs) {
            oss << ptr << " ";
        }*/

        //LOG("-| (%s [%s]) CallObjectMethod: %s [%s] (%p) | %l", JUtil->GetJClassName(env, clazz).c_str(), classSig, methodName, sig, methodID, "");
        LOG("-| (%s) CallObjectMethod: %s [%s] (%p) | %s", JUtil->GetJClassName(env, clazz).c_str(), methodName, sig, methodID, "");//oss.str().c_str());

        //LOG("CallObjectMethod called: %p", lobj);
        QUICK_EXIT:
        jobject result = orig_CallObjectMethod(env, obj, methodID, va_arg(args, void*));
        va_end(args);
        return result;
    }

    //jobject JNICALL Hook_NewGlobalRef(JNIEnv* env, jobject lobj) {
    //    //LOG("NewGlobalRef called: %p", lobj);
    //    return orig_NewGlobalRef(env, lobj);
    //}
    // ========================================================================================================================== //


    // fix and remove args when oop
    NTSTATUS Init(JNIEnv* jenv, jvmtiEnv* jvmtienv) {
        jvmti = jvmtienv;


        orig_RegisterNatives = jenv->functions->RegisterNatives;
        orig_DefineClass = jenv->functions->DefineClass;
        orig_SetByteArrayRegion = jenv->functions->SetByteArrayRegion;

        orig_SetObjectField = jenv->functions->SetObjectField;
        orig_SetLongField = jenv->functions->SetLongField;

        orig_NewString = jenv->functions->NewString;
        orig_NewStringUTF = jenv->functions->NewStringUTF;

        orig_NewGlobalRef = jenv->functions->NewGlobalRef;

        orig_CallObjectMethod = jenv->functions->CallObjectMethod;


        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        
        DetourAttach(&(PVOID&)orig_RegisterNatives, Hook_RegisterNatives);
        DetourAttach(&(PVOID&)orig_DefineClass, Hook_DefineClass);

        DetourAttach(&(PVOID&)orig_SetByteArrayRegion, Hook_SetByteArrayRegion);
        //DetourAttach(&(PVOID&)orig_SetObjectField, Hook_SetObjectField);
        //DetourAttach(&(PVOID&)orig_SetLongField, Hook_SetLongField);

        DetourAttach(&(PVOID&)orig_NewString, Hook_NewString);
        DetourAttach(&(PVOID&)orig_NewStringUTF, Hook_NewStringUTF);

        DetourAttach(&(PVOID&)orig_NewGlobalRef, Hook_NewGlobalRef);

        DetourAttach(&(PVOID&)orig_CallObjectMethod, Hook_CallObjectMethod);

        DetourTransactionCommit();

        LOG("hooked RegisterNatives %p | orig %p", orig_RegisterNatives, Hook_RegisterNatives);
        LOG("hooked DefineClass %p | orig %p", orig_DefineClass, Hook_DefineClass);
        LOG("hooked SetByteArrayRegion %p | orig %p", orig_SetByteArrayRegion, Hook_SetByteArrayRegion);
        LOG("hooked SetObjectField %p | orig %p", orig_SetObjectField, Hook_SetObjectField);
        //LOG("hooked SetLongField %p | orig %p", orig_SetLongField, SetLongFieldHook);
        LOG("hooked NewGlobalRef %p | orig %p", orig_NewGlobalRef, Hook_NewGlobalRef);
        LOG("hooked CallObjectMethod %p | orig %p", orig_CallObjectMethod, Hook_CallObjectMethod);

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