#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <queue>
#include <filesystem>

extern "C" void extractAssets(JNIEnv* env, jclass cls) {
    std::queue<std::string> queue;
    queue.push("");

    jmethodID getContext = env->GetStaticMethodID(cls, "getContext","()Landroid/content/Context;");
    jobject context = env->CallStaticObjectMethod(cls, getContext);

    jclass contextCls = env->GetObjectClass(context);
    jmethodID getAssets = env->GetMethodID(contextCls, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject assetManager = env->CallObjectMethod(context, getAssets);
    jclass amCls = env->GetObjectClass(assetManager);
    jobject gasset = env->NewGlobalRef(assetManager);
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

    jmethodID AssetManager_list = env->GetMethodID(amCls, "list", "(Ljava/lang/String;)[Ljava/lang/String;");

    jmethodID getCacheDir = env->GetMethodID(cls, "getCacheDir", "()Ljava/io/File;");
    jobject file = env->CallObjectMethod(context, getCacheDir);
    jclass fileClass = env->FindClass("java/io/File");
    jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
    const char* app_dir = env->GetStringUTFChars(jpath, NULL);
    std::string cache = app_dir;
    // chdir in the application cache directory
    chdir(app_dir);
    env->ReleaseStringUTFChars(jpath, app_dir);
    // clazz = env->GetObjectClass(env, assetManager);

    while (!queue.empty()) {
        auto dirName = queue.front();
        queue.pop();
        jstring jdirName = env->NewStringUTF(dirName.c_str());
        auto list = (jobjectArray) env->CallObjectMethod(assetManager, AssetManager_list, jdirName);
        if (!list) {
            continue;
        }
        if (!dirName.empty()) {
            std::filesystem::create_directories(cache + "/" + dirName);
        }
        jsize length = env->GetArrayLength(list);
        for (jsize i = 0; i < length; ++i) {
            auto jname = (jstring) env->GetObjectArrayElement(list, i);
            const char* name = env->GetStringUTFChars(jname, NULL);
            std::string fullName = dirName.empty() ? name : dirName + "/" + name;
            env->ReleaseStringUTFChars(jname, name);
            printf("File %d: %s\n", i, fullName.c_str());

            if (auto asset = AAssetManager_open(mgr, fullName.c_str(), AASSET_MODE_STREAMING)) {
                if (auto out = fopen(fullName.c_str(), "wb")) {
                    char buf[1024];
                    int nb_read = 0;
                    while ((nb_read = AAsset_read(asset, buf, sizeof(buf))) > 0)
                        fwrite(buf, nb_read, 1, out);
                    fclose(out);
                }
                AAsset_close(asset);
            } else {
                queue.push(fullName);
            }
        }
    }
    
    // const char* filename;
    // while ((filename = AAssetDir_getNextFileName(assetDir))) {
    // }
    // AAssetDir_close(assetDir);
    env->DeleteGlobalRef(gasset);
}
