#set(ANDROID_ABI "arm64-v8a")#允许arm64-v8a,armeabi-v7a
#set(ANDROID_ABI "armeabi-v7a")#允许arm64-v8a,armeabi-v7a
#set(ANDROID_PLATFORM 28)
#set(CMAKE_TOOLCHAIN_FILE "/home/tony/Android/Sdk/ndk/22.1.7171670/build/cmake/android.toolchain.cmake")#toolchain file 路径
message("ANDROID_ABI==================================================" ${ANDROID_ABI})


#未设置toolchain file

if (${CMAKE_TOOLCHAIN_FILE} STREQUAL "" OR ${ANDROID_ABI} STREQUAL "")
    message("CMAKE_TOOLCHAIN_FILE not found switch start to find compilers path")
    if (NOT ANDROID_ABI)
        message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE doesn't exist, please set ANDROID_ABI in line 1")
    endif ()

    #set(CMAKE_CXX_COMPILER "/home/tony/Android/Sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android28-clang++")
    #set(CMAKE_C_COMPILER "/home/tony/Android/Sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android28-clang")
    set(CMAKE_CXX_COMPILER "/home/tony/Android/Sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi28-clang++")
    set(CMAKE_C_COMPILER "/home/tony/Android/Sdk/ndk/22.1.7171670/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi28-clang")
else ()
    message("CMAKE_TOOLCHAIN_FILE found")
    if (NOT ANDROID_STL)
        set(ANDROID_STL "c++_static")
    endif ()

    # Choose clang if the NDK has both gcc and clang, since gcc sometimes fails
    set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION "clang")
endif ()

if (${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(interceptor_lib_name "interceptor-lib")
    set(LLVM_DIR "Z:\\home\\tony\\aarch64_llvm_hidden_\\lib\\cmake\\llvm")#llvm4.0路径
elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(interceptor_lib_name "interceptor-lib")
    set(LLVM_DIR Z:\\home\\tony\\armv7_llvm_hidden_\\lib\\cmake\\llvm)#llvm4.0路径
else ()
    message(FATAL_ERROR "unknown android abi-- ${ANDROID_ABI}")
endif ()

#全局编译选项
#set(CMAKE_SKIP_RPATH TRUE)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections -ffunction-sections -no-canonical-prefixes -D_FORTIFY_SOURCE=2 -Wformat   -std=c++11   -fvisibility=hidden -fno-rtti -fno-exceptions -v -fPIC -Os")


