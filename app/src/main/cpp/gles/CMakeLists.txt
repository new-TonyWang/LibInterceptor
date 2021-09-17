
set(get_gles_proc_sources
    get_gles_proc_address.cpp
    dl_loader.cpp
    log.cpp

)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(get_gles_proc_lib OBJECT ${get_gles_proc_sources})

target_compile_definitions(get_gles_proc_lib PRIVATE "TARGET_OS_ANDROID")

target_include_directories(get_gles_proc_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


#set_target_properties(get_gles_proc_lib PROPERTIES LINK_FLAGS "-fvisibility=hidden  -Wl,--gc-sections -O3")



