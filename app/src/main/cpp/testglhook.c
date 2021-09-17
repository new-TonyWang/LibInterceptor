//
// Created by ttongyuwang on 2021/8/17.
//
#include <GLES3/gl3.h>
#include <android/log.h>
#include "testglhook.h"

static const char *tag = "rungl~~~~~~~~~~~~~~~~~~~~~";
int rungl() {

        uint32_t mode = 0;
        GLint first = 0;
        GLsizei count = 1;
        __android_log_print(ANDROID_LOG_INFO, tag, "running glDrawArrays");
        glDrawArrays(mode, first, count);


        return 1;
}
int rungl2() {

        uint32_t mode = 0;
        GLint first = 0;
        GLsizei count = 1;
        __android_log_print(ANDROID_LOG_INFO, tag, "running glDrawArrays");
        glDrawArrays(mode, first, count);


        return 1;
}
int rungl3() {

        uint32_t mode = 0;
        GLint first = 0;


        return 1;
}
int rungl4() {

        uint32_t mode = 0;
        GLint first = 0;


        return 1;
}
int rungl5() {

        uint32_t mode = 0;
        GLint first = 0;


        return 1;
}

