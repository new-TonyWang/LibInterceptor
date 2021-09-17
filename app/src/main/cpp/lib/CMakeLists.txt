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
 
add_definitions(${LLVM_DEFINITIONS})

set(lib_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/code_generator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/constant_pool_data_expr.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/disassembler.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/error.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/interceptor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/linker.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/memory_manager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/target.cc
    )

set(interceptor_arch_libs)
message("LLVM_TARGETS_TO_BUILD=" ${LLVM_TARGETS_TO_BUILD})
foreach(TARGET ${LLVM_TARGETS_TO_BUILD})
    file(GLOB arch_sources ${TARGET}/*.cc)
    list(APPEND lib_sources ${arch_sources})

    set(needed_libs
        ${TARGET}AsmParser
        ${TARGET}Desc
        ${TARGET}Disassembler
        ${TARGET}AsmPrinter
        ${TARGET}Info)
  
    if(${TARGET} STREQUAL "AArch64")
        list(APPEND needed_libs ${TARGET}Utils)
    endif()
    
    llvm_map_components_to_libnames(arch_libs ${needed_libs})
    message("arch_libs===" ${arch_libs})
    list(APPEND interceptor_arch_libs ${arch_libs})
endforeach()

llvm_map_components_to_libnames(interceptor_general_libs
    MCDisassembler
    Object
    MCParser
    BitReader
    MC
    Core
    Support)

set(interceptor_libs z ${interceptor_arch_libs} ${interceptor_general_libs})
set(LLVM_LIBS )



foreach(lib ${interceptor_libs})
    message(${lib}"\n")
   
    list(APPEND LLVM_LIBS PRIVATE -l${lib})
endforeach()


# set(tmp_sources)
# foreach(lib ${lib_sources})
#     list(APPEND tmp_sources ${CMAKE_CURRENT_SOURCE_DIR}/${lib})
# endforeach()
# set(lib_sources ${tmp_sources} PARENT_SCOPE)



#add_library(interceptor_lib OBJECT ${lib_sources})



#target_include_directories(interceptor_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#
#target_include_directories(interceptor_lib PRIVATE ${LLVM_INCLUDE_DIRS})

#set_target_properties(interceptor_lib PROPERTIES LINK_FLAGS "-fvisibility=hidden  -Wl,--gc-sections -O3")
set(LLVM_LIBS ${LLVM_LIBS} PARENT_SCOPE)
set(lib_sources ${lib_sources} PARENT_SCOPE)
# add_library(interceptor OBJECT
#     ${lib_sources}
    
#     )

#target_link_directories(interceptor PRIVATE ${LLVM_DIR}/../../)

# target_include_directories(interceptor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# target_include_directories(interceptor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# target_include_directories(interceptor PRIVATE ${LLVM_INCLUDE_DIRS})






