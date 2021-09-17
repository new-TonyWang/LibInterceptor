//
// Created by ttongyuwang on 2021/8/16.
//
#include<android/log.h>
#include <jni.h>
#include "libxhook/xhook.h"
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "testglhook.h"
#include <time.h>
static char* tag = "testxhook-------------------------------";
EGLBoolean myeglSwapBuffers(EGLDisplay display, void *surface)
{
   __android_log_print(ANDROID_LOG_INFO,tag,"HOOKED eglSwapBuffers");
   eglSwapBuffers(display,surface);
    return 0;
}

void myglDrawArrays(uint32_t mode, GLint first, GLsizei count)
{
    __android_log_print(ANDROID_LOG_INFO,tag,"HOOKED glDrawArrays");
        glDrawArrays(mode, first, count);

}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
//    dlopen("/system/lib64/libEGL.so",RTLD_LAZY);
//    xhook_register(".*\\libunity.so", "eglSwapBuffers", myeglSwapBuffers ,  NULL);

    rungl();//第一次调用，没有hook
    dlopen("libtestglhook.so",RTLD_LAZY);
    long start = clock();
    xhook_register(".*\\testxhook.so", "rungl", myglDrawArrays ,  NULL);
    xhook_register(".*\\testxhook.so", "rungl2", myeglSwapBuffers ,  NULL);
    xhook_register(".*\\testxhook.so", "rungl3", myeglSwapBuffers ,  NULL);
    xhook_register(".*\\testxhook.so", "rungl4", myeglSwapBuffers ,  NULL);
    xhook_register(".*\\testxhook.so", "rungl5", myeglSwapBuffers ,  NULL);
    xhook_refresh(0);
    long time_cost = clock()-start;
    __android_log_print(ANDROID_LOG_INFO,tag,"plthook时间：%ld",time_cost);

    //rungl();//第二次调用，已经被hook
return JNI_VERSION_1_4;

}


