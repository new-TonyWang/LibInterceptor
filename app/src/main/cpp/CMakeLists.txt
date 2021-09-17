# Copyright (C) 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#编译器路径，可也可设置toolchain的make文件
#set(CMAKE_TOOLCHAIN_FILE )

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/config.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/config.cmake)
else()
    if(EXISTS ${CMAKE_SOURCE_DIR}/config.cmake)
        include(${CMAKE_SOURCE_DIR}/config.cmake)
    endif()
endif()

project(interceptor-lib_standalone)
cmake_minimum_required(VERSION 3.16)



function(add_source_with_cmake_source_dir sources out)
    foreach(lib  ${soucs})
        list(APPEND out ${CMAKE_CURRENT_SOURCE_DIR}/${lib})
    endforeach()
#全局include
include_directories(${CMAKE_SOURCE_DIR}/include)
endfunction()



find_package(LLVM CONFIG)

if(NOT LLVM_FOUND)
    message(FATAL_ERROR "LLVM not found")
endif()

message("llvm found")
set(LLVM_CCACHE_BUILD OFF)
if(NOT CMAKE_HOST_WIN32)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set(LLVM_CCACHE_BUILD ON)
    endif()
endif()


function(llvm_abi android_abi out)
    if (${android_abi} STREQUAL "arm64-v8a")
        set(${out} "AArch64" PARENT_SCOPE)
    elseif(${android_abi} STREQUAL "armeabi-v7a")
        set(${out} "ARM" PARENT_SCOPE)
    elseif(${android_abi} STREQUAL "x86")
        set(${out} "X86" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unsupported architecture for building LLVM: ${android_abi}")
    endif()
endfunction()

message("ANDROID_ABI for llvm is " ${ANDROID_ABI})
llvm_abi(ANDROID_ABI target_arch)

file(GLOB sources
    "lib/*.h" "lib/*.cc"
    "lib/${target_arch}/*.h" "lib/${target_arch}/*.cc"
)

if (ANDROID_ABI STREQUAL "arm64-v8a")
    set(LLVM_TARGET_ARCH "${target_arch}")
    set(LLVM_HOST_TRIPLE "aarch64-unknown-linux-android")
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(LLVM_TARGET_ARCH "${target_arch}")
    set(LLVM_HOST_TRIPLE "armv8.2a-unknown-linux-android")
elseif(ANDROID_ABI STREQUAL "x86")
    set(LLVM_TARGET_ARCH "${target_arch}")
    set(LLVM_HOST_TRIPLE "i386-unknown-linux-android")
else()
    message(FATAL_ERROR "Unsupported architecture for building LLVM: ${LLVM_TARGET_ARCH}")
endif()
#存放用于编译成单个.so文件的object（例如interceptor-lib和gles）
set(sources)

include_directories(${CMAKE_SOURCE_DIR}/include)
#interceptor-lib相关的代码
add_subdirectory(lib)

#list(APPEND sources $<TARGET_OBJECTS:interceptor_lib>)

link_directories(${LLVM_DIR}/../../)

#get_gles_proc_address相关代码
#add_subdirectory(gles)
#
#list(APPEND sources $<TARGET_OBJECTS:get_gles_proc_lib>)


message("lib_sources=========================" ${lib_sources})
add_library(gpu_interceptor SHARED
        ${lib_sources}
        ./test.cpp
        )

target_include_directories(gpu_interceptor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_include_directories(gpu_interceptor PRIVATE ${LLVM_INCLUDE_DIRS})
find_library(
        log-lib
        log
)

find_path(GLES2_INCLUDE_DIR GLES2/gl2.h
        HINTS ${ANDROID_NDK})              #找到opengl头文件路径
find_library(GLES2_LIBRARY libGLESv2.so
        HINTS ${GLES2_INCLUDE_DIR}/../lib) #找到opengl库文件路径


#add_library(testlib SHARED ./testlib.cpp)

#find_library(testliblib libtestlib.so
#        ) #找到opengl库文件路径
#
#add_library(testlib SHARED IMPORTED)
#
#SET_TARGET_PROPERTIES(testlib PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_SOURCE_DIR}/../libs/arm64-v8a/libtestlib.so
#        )
#link_libraries(testlib)
include_directories( ${CMAKE_SOURCE_DIR}/../libs/arm64-v8a/)





#使用version-script控制函数和符号的导出
#set_target_properties(gpu_interceptor PROPERTIES LINK_FLAGS " -lc++_static -Wl,--gc-sections -Wl,--version-script=${CMAKE_SOURCE_DIR}/export_function.txt")

#set_target_properties(gpu_interceptor PROPERTIES LINK_FLAGS " -lc++_static -Wl,--gc-sections  -Wl,--unresolved-symbols=ignore-all")
#set_target_properties(gpu_interceptor PROPERTIES OUTPUT_NAME gpu_interceptor${interceptor_lib_name}_gdb)


#xhoook

#add_subdirectory(libxhook)
#
#list(APPEND xhook_source testxhook.c)
#
#
#add_library(testglhook SHARED testglhook.c)
#
#set_target_properties(testglhook  PROPERTIES LINK_FLAGS "-llog  -lGLESv3 -lEGL")
#
#
#
#
#add_library(testxhook SHARED ${xhook_source})
#
#target_link_directories(testxhook PRIVATE ./libxhook)
#
#target_link_libraries(testxhook testglhook)
#
#
##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror ")
#
#set_target_properties(testxhook  PROPERTIES LINK_FLAGS "-llog  -lGLESv3 -lEGL")
#find_library(
#        libc++
#        c++lib
#)
find_library(
        lib_c
        c
)
target_link_libraries(gpu_interceptor PRIVATE

        ${LLVM_LIBS}

        ${log-lib}
        ${GLES2_LIBRARY}  #把opengl库文件添加进来
        #${lib_c}

        )
#D:/android/sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/aarch64-linux-android/libc.a
#set_target_properties(gpu_interceptor PROPERTIES LINK_FLAGS "   -Wl,--gc-sections -Wl,--unresolved-symbols=ignore-all -llog -Bdynamic -lGLESv3 -lEGL  -Wl,--version-script=${CMAKE_SOURCE_DIR}/export_function.txt")
set_target_properties(gpu_interceptor PROPERTIES LINK_FLAGS "   -Wl,--gc-sections  -llog -lGLESv3 -lEGL  ")
#set_target_properties(gpu_interceptor PROPERTIES LINK_FLAGS " -lc++_static -Wl,--gc-sections -Wl,--unresolved-symbols=ignore-all")
add_subdirectory(ThreadPool)