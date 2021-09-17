//
// Created by ttongyuwang on 2021/8/1.
#include "testlib.h"
extern "C" {
__attribute__((visibility("default"))) int testfunc(int &a) {
    a = +1;
    return 0;

}
}
