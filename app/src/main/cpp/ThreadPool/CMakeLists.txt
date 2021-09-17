

set(ThreadPoolSource
    thread_pool.cpp
        thread_pool.hpp
        FutureTask.hpp

)

add_library(ThreadPool STATIC ${ThreadPoolSource})