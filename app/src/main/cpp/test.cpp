//
// Created by ttongyuwang on 2021/7/31.
//
#include <jni.h>
#include "interceptor.h"
#include "GLES3/gl3.h"
#include "android/log.h"
#include <android/dlext.h>
#include <dlfcn.h>
#include "testlib.h"
#include <time.h>
#include "testglhook.h"
//unsigned char* a1 = (unsigned char *) "123123";
//GL_APICALL const GLubyte *GL_APIENTRY MyglGetString (GLenum name){
//    __android_log_print(ANDROID_LOG_VERBOSE,"MYglBindTexture","enter hook func------------------------------");
//
//    return a1;
//}
//
void *callback = nullptr;
typedef int (*p) (int &a) ;
void* realfunc;
extern "C" {
void intercept_error(void * error, const char *error_msg) {
    __android_log_print(ANDROID_LOG_ERROR, "MYglBindTexture", "%s", error_msg);
} ;

void error_callback(const char* error ){

}
//void hooktest(int &a) {
//    a = a + 1000;
//   realfunc =reinterpret_cast<p>(callback);
//    realfunc(a);
//
//}

GL_APICALL void GL_APIENTRY myglDrawArrays (GLenum mode, GLint first, GLsizei count){
    typedef void (*real_func) (GLenum,GLint,GLsizei) ;
    real_func func = (real_func)callback;
    func(mode,first,count);
}

JNIEXPORT void JNICALL
Java_com_tencent_interceptor_1lib_1enhanced_MainActivity_hook(JNIEnv *env,jobject instance){
    void *interceptor = InitializeInterceptor();
    GLenum mode = 0;
    GLint first = 0;
    GLsizei count = 1;
    glDrawArrays(mode, first, count);
    long start = clock();

    InterceptFunction(interceptor, (void*)(glDrawArrays), (void *) myglDrawArrays, &callback, &intercept_error, nullptr);
    InterceptFunction(interceptor, (void*)(glDrawArrays), (void *) myglDrawArrays, &callback, &intercept_error, nullptr);
    glDrawArrays(mode, first, count);
}

//int JNI_OnLoad(JavaVM *vm, void *unused) {
//
//    void *interceptor = InitializeInterceptor();
//    android_dlextinfo *info;
////    void *hwHandle = dlopen(nullptr, RTLD_NOW | RTLD_LOCAL);
////    int t2 = 11;
////    testfunc(t2);
////
////    void *fp = dlopen("libtestglhook.so", RTLD_NOW);
//    GLenum mode = 0;
//    GLint first = 0;
//    GLsizei count = 1;
//    glDrawArrays(mode, first, count);
//    long start = clock();
////    void *func = dlsym(fp, "rungl");
////    void *func2 = dlsym(fp, "rungl2");
////    void *func3 = dlsym(fp, "rungl3");
////    void *func4 = dlsym(fp, "rungl4");
////    void *func5 = dlsym(fp, "rungl5");
//
//    InterceptFunction(interceptor, (void*)(glDrawArrays), (void *) myglDrawArrays, &callback, &intercept_error, nullptr);
//    InterceptFunction(interceptor, (void*)(glDrawArrays), (void *) myglDrawArrays, &callback, &intercept_error, nullptr);
////    InterceptFunction(interceptor, func2, (void *) hooktest, &callback, &intercept_error, nullptr);
////    InterceptFunction(interceptor, func3, (void *) hooktest, &callback, &intercept_error, nullptr);
////    InterceptFunction(interceptor, func4, (void *) hooktest, &callback, &intercept_error, nullptr);
////    InterceptFunction(interceptor, func5, (void *) hooktest, &callback, &intercept_error, nullptr);
////    long time_cost = clock()-start;
//    glDrawArrays(mode, first, count);
//    int t = 10;
//    //rungl();
//   // __android_log_print(ANDROID_LOG_INFO,"tag","inlineHook时间：%ld",time_cost);
//    //__android_log_print(ANDROID_LOG_ERROR, "testlib", "%s", t);
//
//    return JNI_VERSION_1_4;
//
//
//}

}

